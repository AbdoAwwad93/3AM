import { NextResponse } from "next/server";
import path from "path";
import fs from "fs/promises";
import { promisify } from "util";
import { execFile, exec } from "child_process";
import crypto from "crypto";
import { processManager } from "@/lib/process-manager";

const execFileAsync = promisify(execFile);
const execAsync = promisify(exec);

// Helper to extract errors (reused from run/route.ts)
function extractErrors(compilerOutput: string, gccOutput: string): string[] {
    const errors: string[] = [];
    const parseLines = compilerOutput.split("\n");
    for (const line of parseLines) {
        if (line.trim().startsWith("Error")) {
            errors.push(line.trim());
        }
    }
    for (const line of parseLines) {
        if (line.includes("[SEMANTIC ERROR]")) {
            errors.push(line.trim());
        }
    }
    if (gccOutput) {
        const gccLines = gccOutput.split("\n");
        for (const line of gccLines) {
            if (line.includes(": error:") || line.includes("undefined reference")) {
                errors.push("System Error: " + line.trim());
            }
        }
    }
    return errors;
}

export async function POST(req: Request) {
    try {
        const body = await req.json();
        const code = typeof body?.code === "string" ? body.code : null;

        if (!code) {
            return NextResponse.json({ error: "No code provided" }, { status: 400 });
        }

        const sessionId = crypto.randomUUID();
        const uploadsDir = path.join(process.cwd(), "uploads", sessionId);
        await fs.mkdir(uploadsDir, { recursive: true });

        const sourcePath = path.join(uploadsDir, "program.3am");
        const cSourcePath = path.join(uploadsDir, "program.c");
        const exePath = path.join(uploadsDir, "program.exe");
        const compilerPath = path.join(process.cwd(), "public", "compiler_v2.exe");

        // 1. Save 3AM code
        await fs.writeFile(sourcePath, code, "utf8");

        let compilerStdout = "";
        let compilerStderr = "";
        let gccStderr = "";

        try {
            // 2. Compile 3AM to C
            const compResult = await execFileAsync(compilerPath, [sourcePath, cSourcePath], { timeout: 10000 });
            compilerStdout = compResult.stdout;
            compilerStderr = compResult.stderr;

            // Check for compiler errors
            const combinedOutput = compilerStdout + "\n" + compilerStderr;
            if (combinedOutput.includes("Error") || combinedOutput.includes("[SEMANTIC ERROR]")) {
                return NextResponse.json({
                    error: "Compilation Failed",
                    errors: extractErrors(combinedOutput, ""),
                    compilerOutput: combinedOutput
                }, { status: 400 });
            }

            // 3. Compile C to EXE
            try {
                // Use execAsync for gcc to handle shell command
                await execAsync(`gcc "${cSourcePath}" -o "${exePath}"`, { timeout: 10000 });
            } catch (gccErr: any) {
                gccStderr = gccErr.stderr || gccErr.message;
                return NextResponse.json({
                    error: "System Compilation Failed",
                    errors: extractErrors(compilerStdout + "\n" + compilerStderr, gccStderr),
                    compilerOutput: compilerStdout + "\n" + compilerStderr,
                    gccOutput: gccStderr
                }, { status: 400 });
            }

            // 4. Start interactive process
            processManager.start(sessionId, exePath, [], uploadsDir);

            return NextResponse.json({
                sessionId: sessionId,
                message: "Process started"
            });

        } catch (compErr: any) {
            const combinedErr = (compErr.stdout || "") + "\n" + (compErr.stderr || "");
            return NextResponse.json({
                error: "3AM Compiler Error",
                errors: extractErrors(combinedErr, ""),
                compilerOutput: combinedErr
            }, { status: 400 });
        }
    } catch (err: any) {
        return NextResponse.json({ error: String(err) }, { status: 500 });
    }
}
