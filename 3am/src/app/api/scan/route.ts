// app/api/scan/route.ts
import { NextResponse } from "next/server";
import path from "path";
import fs from "fs/promises";
import { promisify } from "util";
import { execFile } from "child_process";
import crypto from "crypto";

const execFileAsync = promisify(execFile);

export async function POST(req: Request) {
  try {
    // Expect JSON { code: string }
    const body = await req.json();
    const code = typeof body?.code === "string" ? body.code : null;

    if (!code) {
      return NextResponse.json({ error: "No code provided" }, { status: 400 });
    }

    // Ensure uploads folder exists (server-side)
    const uploadsDir = path.join(process.cwd(), "uploads");
    await fs.mkdir(uploadsDir, { recursive: true });

    // Unique filename to avoid collisions
    const filename = `${crypto.randomUUID()}.txt`;
    const filePath = path.join(uploadsDir, filename);

    // Save code to temporary file
    await fs.writeFile(filePath, code, "utf8");

    // Path to the scanner executable (placed in public or another folder)
    // Use a fixed, trusted path—do NOT accept exe path from user.
    const exePath = path.join(process.cwd(), "public", "scanner.exe");

    // Run the executable with the file path as argument
    let stdout = "";
    let stderr = "";
    try {
      const result = await execFileAsync(exePath, [filePath], {
        timeout: 30_000,
      }); // timeout in ms
      // result is usually { stdout, stderr }
      stdout = (result as any).stdout ?? "";
      stderr = (result as any).stderr ?? "";
    } catch (execErr: any) {
      // execFile throws on non-zero exit or spawn errors.
      // capture any stdout/stderr available on the error object
      stderr = execErr?.stderr ?? execErr?.message ?? String(execErr);
      stdout = execErr?.stdout ?? "";
      // Clean up file below in finally
    } finally {
      // Remove the temporary file
      try {
        await fs.unlink(filePath);
      } catch {
        /* ignore unlink errors */
      }
    }

    return NextResponse.json({ output: stdout + "\n" + stderr });
  } catch (err: any) {
    return NextResponse.json({ error: String(err) }, { status: 500 });
  }
}
