import { NextResponse } from "next/server";
import { processManager } from "@/lib/process-manager";

export async function POST(req: Request) {
    try {
        const body = await req.json();
        const { sessionId } = body;

        if (sessionId) {
            processManager.kill(sessionId);
        }

        return NextResponse.json({ success: true });
    } catch (err) {
        return NextResponse.json({ error: String(err) }, { status: 500 });
    }
}
