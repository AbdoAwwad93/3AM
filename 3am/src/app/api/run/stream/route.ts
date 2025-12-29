import { NextResponse } from "next/server";
import { processManager } from "@/lib/process-manager";

export const dynamic = 'force-dynamic';

export async function GET(req: Request) {
    const { searchParams } = new URL(req.url);
    const sessionId = searchParams.get("id");

    if (!sessionId) {
        return NextResponse.json({ error: "Session ID required" }, { status: 400 });
    }

    const proc = processManager.get(sessionId);
    if (!proc) {
        return NextResponse.json({ error: "Process not found" }, { status: 404 });
    }

    const stream = new ReadableStream({
        start(controller) {
            const encoder = new TextEncoder();

            const sendEvent = (event: string, data: any) => {
                const message = `event: ${event}\ndata: ${JSON.stringify(data)}\n\n`;
                controller.enqueue(encoder.encode(message));
            };

            // Send buffered output first
            if (proc.outputBuffer.length > 0) {
                for (const msg of proc.outputBuffer) {
                    sendEvent("output", msg);
                }
            }

            const onOutput = (data: { type: string, text: string }) => {
                sendEvent("output", data);
            };

            const onExit = (code: number) => {
                sendEvent("exit", { code });
                controller.close();
            };

            proc.emitter.on("output", onOutput);
            proc.emitter.on("exit", onExit);

            if (proc.isExited) {
                sendEvent("exit", { code: proc.exitCode });
                controller.close();
            }

            // Cleanup listener on stream close
            req.signal.addEventListener("abort", () => {
                proc.emitter.off("output", onOutput);
                proc.emitter.off("exit", onExit);
            });
        }
    });

    return new Response(stream, {
        headers: {
            "Content-Type": "text/event-stream",
            "Cache-Control": "no-cache",
            "Connection": "keep-alive",
        },
    });
}
