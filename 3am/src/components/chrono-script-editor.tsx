"use client";
import { useState, useRef } from "react";
import type React from "react";

import { Play, Copy, Check, AlertCircle, Terminal, FileCode, CheckCircle, Keyboard, Activity, AlertTriangle, Square } from "lucide-react";

// ChronoScript language keywords and data
const CHRONOSCRIPT_KEYWORDS = {
  keywords: [
    "startClock",
    "schedule",
    "tickout",
    "tickin",
    "when",
    "otherwise",
    "repeat",
    "loop",
    "finish",
    "import",
    "timeline",
  ],
  types: ["second", "minute", "moment", "flag"],
  examples: [
    "startClock() { ... }",
    "schedule alarm() { ... }",
    "tickout 'message'",
    "tickin variable",
    "when (condition)",
    "repeat (condition)",
    "loop (init; condition; increment)",
  ],
};

interface Suggestion {
  text: string;
  type: "keyword" | "type" | "example";
}

export default function ChronoScriptEditor() {
  const [code, setCode] = useState<string>("");
  const [loading, setLoading] = useState(false);
  const [output, setOutput] = useState<string>("");
  const [terminalOutput, setTerminalOutput] = useState<Array<{ type: 'stdout' | 'stderr' | 'system', text: string }>>([]);
  const [sessionId, setSessionId] = useState<string | null>(null);
  const [terminalInput, setTerminalInput] = useState("");
  const [isRunning, setIsRunning] = useState(false);
  const [errors, setErrors] = useState<Array<{ message: string; line: number; type: "syntax" | "semantic" | "system" }>>([]);

  const [suggestions, setSuggestions] = useState<Suggestion[]>([]);
  const [showSuggestions, setShowSuggestions] = useState(false);
  const [selectedSuggestion, setSelectedSuggestion] = useState(0);
  const textareaRef = useRef<HTMLTextAreaElement>(null);
  const lineNumbersRef = useRef<HTMLDivElement>(null);
  const suggestionsRef = useRef<HTMLDivElement>(null);

  // Handle autocomplete
  const handleInputChange = (e: React.ChangeEvent<HTMLTextAreaElement>) => {
    const value = e.target.value;
    setCode(value);

    const lines = value.split("\n");
    const currentLine = lines[lines.length - 1];
    const words = currentLine.split(/\s+/);
    const lastWord = words[words.length - 1].toLowerCase();

    if (lastWord.length > 0) {
      const filtered: Suggestion[] = [];
      CHRONOSCRIPT_KEYWORDS.keywords.forEach((kw) => {
        if (kw.toLowerCase().startsWith(lastWord)) filtered.push({ text: kw, type: "keyword" });
      });
      CHRONOSCRIPT_KEYWORDS.types.forEach((type) => {
        if (type.toLowerCase().startsWith(lastWord)) filtered.push({ text: type, type: "type" });
      });
      CHRONOSCRIPT_KEYWORDS.examples.forEach((ex) => {
        if (ex.toLowerCase().startsWith(lastWord)) filtered.push({ text: ex, type: "example" });
      });
      setSuggestions(filtered);
      setShowSuggestions(filtered.length > 0);
      setSelectedSuggestion(0);
    } else {
      setShowSuggestions(false);
      setSuggestions([]);
    }
  };

  const handleKeyDown = (e: React.KeyboardEvent<HTMLTextAreaElement>) => {
    if (!showSuggestions) return;
    switch (e.key) {
      case "ArrowDown":
        e.preventDefault();
        setSelectedSuggestion((prev) => (prev < suggestions.length - 1 ? prev + 1 : prev));
        break;
      case "ArrowUp":
        e.preventDefault();
        setSelectedSuggestion((prev) => (prev > 0 ? prev - 1 : 0));
        break;
      case "Enter":
        e.preventDefault();
        if (suggestions[selectedSuggestion]) insertSuggestion(suggestions[selectedSuggestion].text);
        break;
      case "Escape":
        setShowSuggestions(false);
        break;
    }
  };

  const insertSuggestion = (suggestion: string) => {
    const textarea = textareaRef.current;
    if (!textarea) return;
    const lines = code.split("\n");
    const currentLine = lines[lines.length - 1];
    const words = currentLine.split(/\s+/);
    const lastWord = words[words.length - 1];
    const newCode = code.slice(0, -lastWord.length) + suggestion + " ";
    setCode(newCode);
    setShowSuggestions(false);
    setTimeout(() => textarea.focus(), 0);
  };

  async function handleScan() {
    setLoading(true);
    setErrors([]);
    setOutput("");
    try {
      const res = await fetch("/api/scan", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ code }),
      });
      const data = await res.json();
      if (!res.ok) setErrors([{ message: data.error || "Scanning failed", line: 0, type: "system" }]);
      else setOutput(data.output || "");
    } catch (err) {
      setErrors([{ message: String(err), line: 0, type: "system" }]);
    } finally {
      setLoading(false);
    }
  }

  const handleRun = async () => {
    if (isRunning) return;

    // Clear previous output/errors
    setErrors([]);
    setTerminalOutput([]);
    setIsRunning(true);

    try {
      // 1. Start Process
      const startRes = await fetch("/api/run/start", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ code }),
      });

      const startData = await startRes.json();

      if (!startRes.ok) {
        if (startData.errors) {
          setErrors(startData.errors.map((err: string) => {
            const match = err.match(/Error at line (\d+): (.*)/) || err.match(/Error: (.*)/) || err.match(/\[SEMANTIC ERROR\] Line (\d+): (.*)/) || [null, "0", err];
            let line = 0;
            let message = err;
            let type: "syntax" | "semantic" | "system" = "system";

            if (err.includes("[SEMANTIC ERROR]")) {
              type = "semantic";
              const match = err.match(/Line (\d+): (.*)/);
              if (match) {
                line = parseInt(match[1]);
                message = match[2];
              }
            } else if (err.startsWith("Error")) {
              type = "syntax";
              const match = err.match(/at line (\d+): (.*)/);
              if (match) {
                line = parseInt(match[1]);
                message = match[2];
              } else {
                message = err.replace("Error:", "").trim();
              }
            }

            return { message, line, type };
          }));
        } else {
          setTerminalOutput([{ type: 'system', text: startData.error || "Failed to start process" }]);
        }
        setIsRunning(false);
        return;
      }

      setSessionId(startData.sessionId);

      // 2. Connect to Event Stream
      const eventSource = new EventSource(`/api/run/stream?id=${startData.sessionId}`);

      // Standard message handler (fallback)
      eventSource.onmessage = (event) => {
        // debug log or handle parsing if needed
      };

      // Custom event listeners for SSE
      eventSource.addEventListener("output", (event: any) => {
        try {
          const data = JSON.parse(event.data);
          setTerminalOutput(prev => [...prev, { type: data.type, text: data.text }]);
        } catch (e) { }
      });

      eventSource.addEventListener("exit", (event: any) => {
        const data = JSON.parse(event.data);
        setTerminalOutput(prev => [...prev, { type: 'system', text: `\nProcess exited with code ${data.code}` }]);
        setIsRunning(false);
        setSessionId(null);
        eventSource.close();
      });

      eventSource.onerror = () => {
        eventSource.close();
        setIsRunning(false);
        setSessionId(null);
      };

    } catch (err) {
      setTerminalOutput([{ type: 'system', text: "An error occurred starting the process." }]);
      setIsRunning(false);
    }
  };

  const handleStop = async () => {
    if (sessionId) {
      await fetch("/api/run/stop", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ sessionId }),
      });
    }
  };

  const handleTerminalInput = async (e: React.KeyboardEvent<HTMLInputElement>) => {
    if (e.key === 'Enter' && sessionId) {
      const input = terminalInput + "\n";
      setTerminalOutput(prev => [...prev, { type: 'stdout', text: input }]); // Echo local input
      setTerminalInput("");

      await fetch("/api/run/input", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ sessionId, input }),
      });
    }
  };

  const handleScroll = () => {
    if (textareaRef.current && lineNumbersRef.current) {
      lineNumbersRef.current.scrollTop = textareaRef.current.scrollTop;
    }
  };


  function extractSection(output: string, section: "LEXICAL" | "SYNTAX" | "SEMANTIC") {
    if (!output) return "";
    const start = output.indexOf(`--- PHASE: ${section}`);
    if (start === -1) return "";
    const end = output.indexOf("--- PHASE:", start + 1);
    return output.substring(start, end === -1 ? output.length : end).trim();
  }

  return (
    <div className="min-h-screen bg-slate-950 text-slate-200">
      {/* Header */}
      <header className="border-b border-slate-800 bg-slate-950/80 backdrop-blur-md sticky top-0 z-50">
        <div className="max-w-7xl mx-auto px-6 py-3 flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-9 h-9 rounded-lg bg-gradient-to-tr from-violet-600 to-indigo-500 shadow-lg shadow-indigo-500/20 flex items-center justify-center">
              <span className="text-white font-bold text-lg">⏰</span>
            </div>
            <div>
              <h1 className="text-lg font-bold text-white tracking-tight">3AM IDE</h1>
              <p className="text-[10px] text-slate-400 font-medium uppercase tracking-wider">Time-Based Compiler</p>
            </div>
          </div>
          <div className="flex gap-4">
            <button onClick={handleRun} disabled={isRunning || !code.trim()} className="h-9 px-4 bg-violet-600 hover:bg-violet-700 disabled:opacity-50 disabled:cursor-not-allowed text-white text-sm font-semibold rounded-md flex items-center gap-2 transition-all">
              {isRunning ? <><Activity size={14} className="animate-spin" /> Running...</> : <><Play size={14} fill="currentColor" /> Run</>}
            </button>
            <button onClick={() => { setCode(""); setErrors([]); setTerminalOutput([]); }} className="h-9 px-4 bg-slate-800 hover:bg-slate-700 text-slate-300 text-sm font-semibold rounded-md transition-all">
              Clear
            </button>
          </div>
        </div>
      </header>

      <main className="max-w-7xl mx-auto p-6 grid grid-cols-1 lg:grid-cols-12 gap-6 h-[calc(100vh-80px)]">
        {/* Left Column: Editor & Stdin */}
        <div className="lg:col-span-7 flex flex-col gap-4">
          <div className="flex-1 flex flex-col bg-slate-900/50 rounded-xl border border-slate-800 overflow-hidden shadow-xl">
            <div className="px-4 py-2 border-b border-slate-800 bg-slate-900/80 flex items-center justify-between shrink-0">
              <span className="text-xs font-bold text-slate-400 flex items-center gap-2"><FileCode size={14} /> main.3am</span>
            </div>
            <div className="relative flex-1 min-h-0">
              <div
                ref={lineNumbersRef}
                className="absolute left-0 top-0 bottom-0 w-10 bg-slate-950/50 border-r border-slate-800/50 flex flex-col items-center pt-4 text-[10px] text-slate-600 font-mono select-none overflow-hidden"
              >
                {code.split("\n").map((_, i) => (
                  <div key={i} className="h-6 leading-6">{i + 1}</div>
                ))}
              </div>
              <textarea
                ref={textareaRef}
                value={code}
                onChange={handleInputChange}
                onKeyDown={handleKeyDown}
                onScroll={handleScroll}
                spellCheck="false"
                placeholder="# Start coding here..."
                className="w-full h-full pl-12 pr-4 py-4 bg-transparent font-mono text-sm text-slate-100 outline-none resize-none placeholder-slate-700 leading-6"
              />
              {showSuggestions && suggestions.length > 0 && (
                <div ref={suggestionsRef} className="absolute left-12 top-10 bg-slate-800 border border-slate-700 rounded-lg shadow-2xl z-50 w-64 divide-y divide-slate-700/50 overflow-hidden">
                  {suggestions.map((suggestion, index) => (
                    <button key={index} onClick={() => insertSuggestion(suggestion.text)} className={`w-full text-left px-3 py-2 text-xs font-mono transition-colors flex items-center justify-between ${index === selectedSuggestion ? "bg-violet-600 text-white" : "text-slate-400 hover:bg-slate-700/50"}`}>
                      <span>{suggestion.text}</span>
                      <span className="text-[10px] opacity-60 px-1.5 py-0.5 rounded border border-current">{suggestion.type}</span>
                    </button>
                  ))}
                </div>
              )}
            </div>
          </div>
        </div>

        {/* Right Column: Console & Errors */}
        <div className="lg:col-span-5 flex flex-col gap-4">

          {/* Errors Section */}
          {errors.length > 0 && (
            <div className="bg-red-950/30 rounded-xl border border-red-900/50 overflow-hidden shadow-xl animate-in fade-in slide-in-from-top-4 duration-300">
              <div className="px-4 py-2 bg-red-900/20 border-b border-red-900/30 flex items-center gap-2">
                <AlertTriangle size={14} className="text-red-400" />
                <span className="text-xs font-bold text-red-200">Compilation Errors</span>
              </div>
              <div className="p-2 max-h-40 overflow-y-auto custom-scrollbar">
                {errors.map((err, i) => (
                  <div
                    key={i}
                    className="flex items-start gap-3 p-2 rounded-lg hover:bg-red-900/20 transition-colors cursor-pointer group"
                    onClick={() => {
                      if (err.line > 0 && textareaRef.current) {
                        const lineHeight = 24;
                        textareaRef.current.scrollTo({ top: (err.line - 1) * lineHeight - 50, behavior: 'smooth' });
                      }
                    }}
                  >
                    <div className="mt-0.5 w-5 h-5 rounded-md bg-red-500/10 flex items-center justify-center shrink-0 border border-red-500/20 group-hover:border-red-500/40 transition-colors">
                      <span className="text-[10px] font-mono font-bold text-red-400">{err.line || "!"}</span>
                    </div>
                    <div className="flex-1 min-w-0">
                      <p className="text-xs text-red-200 font-medium leading-relaxed">{err.message}</p>
                      <p className="text-[10px] text-red-400/60 mt-0.5 uppercase tracking-wider font-bold">{err.type}</p>
                    </div>
                  </div>
                ))}
              </div>
            </div>
          )}

          {/* Terminal */}
          <div className="flex-1 flex flex-col bg-slate-900/50 rounded-xl border border-slate-800 overflow-hidden shadow-xl min-h-[300px]">
            <div className="px-4 py-2 border-b border-slate-800 bg-slate-900/80 flex items-center justify-between">
              <span className="text-xs font-bold text-slate-400 flex items-center gap-2">
                <Terminal size={14} /> Terminal
              </span>
              <div className="flex items-center gap-2">
                {isRunning ? (
                  <button
                    onClick={handleStop}
                    className="p-1 hover:bg-red-500/10 text-red-400 rounded-md transition-colors"
                    title="Stop Execution"
                  >
                    <Square size={12} fill="currentColor" />
                  </button>
                ) : (
                  <div className="w-2 h-2 rounded-full bg-slate-700" title="Idle" />
                )}
              </div>
            </div>

            <div className="flex-1 p-4 font-mono text-sm overflow-y-auto custom-scrollbar bg-black/40">
              {terminalOutput.map((line, i) => (
                <span key={i} className={`${line.type === 'stderr' || line.type === 'system' ? 'text-red-400' : 'text-slate-300'} whitespace-pre-wrap`}>
                  {line.text}
                </span>
              ))}
              {isRunning && (
                <div className="flex items-center gap-2 mt-2">
                  <span className="text-green-500">➜</span>
                  <input
                    type="text"
                    className="flex-1 bg-transparent border-none outline-none text-slate-100 placeholder-slate-600 focus:ring-0"
                    placeholder="Type input..."
                    value={terminalInput}
                    onChange={(e) => setTerminalInput(e.target.value)}
                    onKeyDown={handleTerminalInput}
                    autoFocus
                  />
                </div>
              )}
              {!isRunning && terminalOutput.length === 0 && errors.length === 0 && (
                <div className="text-slate-700 italic">Ready for input...</div>
              )}
            </div>
          </div>
        </div>
      </main>
    </div>
  );
}
