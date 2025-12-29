// app/api/run/route.ts
import { NextResponse } from "next/server";
import path from "path";
import fs from "fs/promises";
import { promisify } from "util";
import { execFile, exec } from "child_process";
import crypto from "crypto";

const execFileAsync = promisify(execFile);
const execAsync = promisify(exec);

function extractErrors(compilerOutput: string, gccOutput: string): string[] {
    const errors: string[] = [];

    // 1. Scan/Parse Errors (usually start with "Error:")
    const parseLines = compilerOutput.split("\n");
    for (const line of parseLines) {
        if (line.trim().startsWith("Error")) {
            errors.push(line.trim());
        }
    }

    // 2. Semantic Errors (usually start with "[SEMANTIC ERROR]")
    for (const line of parseLines) {
        if (line.includes("[SEMANTIC ERROR]")) {
            errors.push(line.trim());
        }
    }

    // 3. GCC/Linker Errors
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
        const input = typeof body?.input === "string" ? body.input : "";

        if (!code) {
            return NextResponse.json({ error: "No code provided" }, { status: 400 });
        }

        const sessionId = crypto.randomUUID();
        const uploadsDir = path.join(process.cwd(), "uploads", sessionId);
        await fs.mkdir(uploadsDir, { recursive: true });

        const sourcePath = path.join(uploadsDir, "program.3am");
        const cSourcePath = path.join(uploadsDir, "program.c");
        const exePath = path.join(uploadsDir, "program.exe");
        const compilerPath = path.join(process.cwd(), "public", "compiler.exe");

        // 1. Save 3AM code
        await fs.writeFile(sourcePath, code, "utf8");

        let compilerStdout = "";
        let compilerStderr = "";
        let runtimeStdout = "";
        let runtimeStderr = "";
        let gccStderr = "";

        try {
            // 2. Compile 3AM to C
            const compResult = await execFileAsync(compilerPath, [sourcePath, cSourcePath], { timeout: 10000 });
            compilerStdout = compResult.stdout;
            compilerStderr = compResult.stderr;

            // Check for compiler errors in stdout/stderr even if it didn't "fail" with crash
            const combinedOutput = compilerStdout + "\n" + compilerStderr;
            if (combinedOutput.includes("Error:") || combinedOutput.includes("[SEMANTIC ERROR]")) {
                return NextResponse.json({
                    error: "Compilation Failed",
                    errors: extractErrors(combinedOutput, ""),
                    compilerOutput: combinedOutput
                }, { status: 400 });
            }

            // 3. Compile C to EXE
            try {
                const gccResult = await execAsync(`gcc "${cSourcePath}" -o "${exePath}"`, { timeout: 10000 });
            } catch (gccErr: any) {
                gccStderr = gccErr.stderr || gccErr.message;
                return NextResponse.json({
                    error: "System Compilation Failed",
                    errors: extractErrors(compilerStdout + "\n" + compilerStderr, gccStderr),
                    compilerOutput: compilerStdout + "\n" + compilerStderr,
                    gccOutput: gccStderr
                }, { status: 400 });
            }

            // 4. Run the EXE
            try {
                const inputPath = path.join(uploadsDir, "input.txt");
                await fs.writeFile(inputPath, input, "utf8");

                const runResult = await execAsync(`"${exePath}" < "${inputPath}"`, { timeout: 5000 });
                runtimeStdout = runResult.stdout;
                runtimeStderr = runResult.stderr;
            } catch (runErr: any) {
                runtimeStdout = runErr.stdout || "";
                runtimeStderr = runErr.stderr || runErr.message || "Execution error";
            }

        } catch (compErr: any) {
            const combinedErr = (compErr.stdout || "") + "\n" + (compErr.stderr || "");
            return NextResponse.json({
                error: "3AM Compiler Error",
                errors: extractErrors(combinedErr, ""),
                compilerOutput: combinedErr
            }, { status: 400 });
        } finally {
            // Cleanup uploads
            try {
                // await fs.rm(uploadsDir, { recursive: true, force: true });
            } catch { }
        }

        return NextResponse.json({
            compilerOutput: compilerStdout + "\n" + compilerStderr,
            output: runtimeStdout,
            stderr: runtimeStderr
        });

    } catch (err: any) {
        return NextResponse.json({ error: String(err) }, { status: 500 });
    }
}
