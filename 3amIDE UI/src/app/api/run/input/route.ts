import { NextResponse } from "next/server";
import { processManager } from "@/lib/process-manager";

export async function POST(req: Request) {
    try {
        const body = await req.json();
        const { sessionId, input } = body;

        if (!sessionId || input === undefined) {
            return NextResponse.json({ error: "Missing sessionId or input" }, { status: 400 });
        }

        const success = processManager.write(sessionId, input);

        if (!success) {
            return NextResponse.json({ error: "Process not active or write failed" }, { status: 400 });
        }

        return NextResponse.json({ success: true });
    } catch (err) {
        return NextResponse.json({ error: String(err) }, { status: 500 });
    }
}
