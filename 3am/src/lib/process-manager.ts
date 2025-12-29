import { spawn, ChildProcess } from "child_process";
import { EventEmitter } from "events";

interface ActiveProcess {
    id: string;
    process: ChildProcess;
    outputBuffer: { type: "stdout" | "stderr" | "system", text: string }[];
    emitter: EventEmitter;
    isExited: boolean;
    exitCode?: number | null;
}

class ProcessManager {
    private processes: Map<string, ActiveProcess> = new Map();

    start(id: string, command: string, args: string[], cwd: string) {
        // Kill existing process with same ID if any
        if (this.processes.has(id)) {
            this.kill(id);
        }

        const emitter = new EventEmitter();
        const child = spawn(command, args, { cwd, shell: false });

        const proc: ActiveProcess = {
            id,
            process: child,
            outputBuffer: [],
            emitter,
            isExited: false,
        };

        child.stdout?.on("data", (data) => {
            const text = data.toString();
            const msg = { type: "stdout" as const, text };
            proc.outputBuffer.push(msg);
            emitter.emit("output", msg);
        });

        child.stderr?.on("data", (data) => {
            const text = data.toString();
            const msg = { type: "stderr" as const, text };
            proc.outputBuffer.push(msg);
            emitter.emit("output", msg);
        });

        child.on("close", (code) => {
            proc.isExited = true;
            proc.exitCode = code;
            emitter.emit("exit", code);
            // Cleanup after a delay to allow final reads
            setTimeout(() => {
                this.processes.delete(id);
            }, 60000); // Keep metadata for 1 min
        });

        child.on("error", (err) => {
            const text = `System Error: ${err.message}\n`;
            const msg = { type: "stderr" as const, text };
            proc.outputBuffer.push(msg);
            emitter.emit("output", msg);
        });

        this.processes.set(id, proc);
        return proc;
    }

    write(id: string, input: string) {
        const proc = this.processes.get(id);
        if (proc && !proc.isExited && proc.process.stdin) {
            proc.process.stdin.write(input);
            // Echo input to output for visibility (optional, depending on terminal behavior)
            // proc.emitter.emit("output", { type: "stdin", text: input }); 
            return true;
        }
        return false;
    }

    kill(id: string) {
        const proc = this.processes.get(id);
        if (proc && !proc.isExited) {
            try {
                proc.process.kill();
            } catch (err) {
                console.error(`Failed to kill process ${id}:`, err);
            }
        }
        this.processes.delete(id);
    }

    get(id: string) {
        return this.processes.get(id);
    }
}

// Singleton instance
// Singleton instance with HMR support
const globalForPM = globalThis as unknown as { processManagerV2: ProcessManager };

export const processManager = globalForPM.processManagerV2 || new ProcessManager();

if (process.env.NODE_ENV !== "production") globalForPM.processManagerV2 = processManager;
