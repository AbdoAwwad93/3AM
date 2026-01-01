import Link from "next/link";
import {
  Clock,
  ArrowLeft,
  Code,
  Zap,
  BookOpen,
  Terminal,
  ArrowRight,
} from "lucide-react";

/* =========================
   DATA
========================= */

const keywords = [
  {
    keyword: "startClock()",
    concept: "Program start",
    example: "startClock() { ... }",
  },
  {
    keyword: "schedule",
    concept: "Function definition",
    example: "schedule alarm() { ... }",
  },
  {
    keyword: "second, minute, hour, name, flag",
    concept: "Variable declaration",
    example: "minute countdown = 5;",
  },
  { keyword: "tickout", concept: "Output", example: 'tickout "Time\'s up!";' },
  { keyword: "tickin", concept: "Input", example: "tickin userTime;" },
  { keyword: "when", concept: "If", example: "when (hour >= 12)" },
  { keyword: "otherwise", concept: "Else", example: "otherwise { ... }" },
  { keyword: "repeat", concept: "While", example: "repeat (minute < 60)" },
  {
    keyword: "loop",
    concept: "For",
    example: "loop (second i = 0; i < 10; i++)",
  },
  { keyword: "TimeUP", concept: "Break", example: "TimeUP;" },
  { keyword: "nexttick", concept: "Continue", example: "nexttick;" },
  { keyword: "finish", concept: "Return", example: "finish value;" },
  { keyword: "#import", concept: "Include", example: "#import calendar" },
  { keyword: "timeline", concept: "Namespace", example: "timeline system" },
  { keyword: "#", concept: "Comment", example: "# this is a comment" },
];

const dataTypes = [
  { type: "second", meaning: "Integer value", example: "second ticks = 30;" },
  {
    type: "minute",
    meaning: "Double value",
    example: "minute timeSpent = 12.5;",
  },
  {
    type: "moment",
    meaning: "String value",
    example: 'moment period = "Morning";',
  },
  { type: "flag", meaning: "Boolean flag", example: "flag alarmSet = true;" },
];

const codeExamples = [
  {
    title: "Hello World — The First Tick",
    code: `#startclock timeline

schedule start() {
  tickout "Clock started — Welcome to 3AM!";
  finish 0;
}`,
  },
  {
    title: "Variables and Basic I/O",
    code: `#startclock timeline

schedule start() {
  moment name;
  second age;

  tickout "Enter your name: ";
  tickin name;

  tickout "Enter age: ";
  tickin age;

  tickout "At " + name + ", " + age + " second have passed.";
  finish 0;
}`,
  },
  {
    title: "Condition (Time Check)",
    code: `#startclock timeline

schedule start() {
  second elapsed = 12;
  when (elapsed >= 10) {
    tickout "Clock is running smoothly!";
  }
  otherwise {
    tickout "Clock hasn't started yet.";
  }
}`,
  },
  {
    title: "Loop (Clock Ticking)",
    code: `#startclock timeline

schedule start() {
  loop (second i = 0; i < 5; i++) {
    tickout "Tick " + i;
  }
}`,
  },
  {
    title: "Function Example (Start Timer)",
    code: `#startclock timeline

schedule startTimer(second timeLeft) {
  tickout "Timer started...";
  repeat (timeLeft > 0) {
    tickout "Tick... " + timeLeft;
    timeLeft = timeLeft - 1;
  }
  tickout "Timer finished!";
  finish 0;
}

schedule start() {
  startTimer(5);
  finish 0;
}`,
  },
];

/* =========================
   SYNTAX HIGHLIGHTER
========================= */

function highlightLine(line: string) {
  const tokenRegex =
    /(#startclock|#import|schedule|tickout|tickin|when|otherwise|repeat|loop|finish|second|minute|moment|flag|timeline|"[^"]*"|\b\d+\b)/g;

  const parts = line.split(tokenRegex);

  return parts.map((part, i) => {
    if (!part) return null;

    if (part.startsWith('"'))
      return (
        <span key={i} className="text-green-400">
          {part}
        </span>
      );

    if (part === "#startclock" || part === "#import")
      return (
        <span key={i} className="text-fuchsia-400">
          {part}
        </span>
      );

    if (
      /^(schedule|tickout|tickin|when|otherwise|repeat|loop|finish|second|minute|moment|flag)$/.test(
        part
      )
    )
      return (
        <span key={i} className="text-violet-400">
          {part}
        </span>
      );

    if (part === "timeline")
      return (
        <span key={i} className="text-cyan-400">
          {part}
        </span>
      );

    if (/^\d+$/.test(part))
      return (
        <span key={i} className="text-orange-400">
          {part}
        </span>
      );

    return <span key={i}>{part}</span>;
  });
}

/* =========================
   PAGE
========================= */

export default function AboutPage() {
  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950">
      {/* Header */}
      <header className="border-b border-slate-800 bg-slate-950/50 backdrop-blur-sm sticky top-0 z-50">
        <div className="max-w-7xl mx-auto px-6 py-4 flex items-center justify-between">
          <Link href="/" className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-lg bg-gradient-to-br from-violet-500 to-fuchsia-500 flex items-center justify-center">
              <Clock className="w-6 h-6 text-white" />
            </div>
            <div>
              <h1 className="text-xl font-bold text-white">3AM</h1>
              <p className="text-xs text-slate-400">Documentation</p>
            </div>
          </Link>
        </div>
      </header>

      {/* Content */}
      <main className="max-w-5xl mx-auto px-6 py-12">
        <Link
          href="/"
          className="inline-flex items-center gap-2 text-slate-400 hover:text-white mb-8"
        >
          <ArrowLeft className="w-4 h-4" />
          Back to Home
        </Link>

        {/* Code Examples */}
        <section className="space-y-6">
          {codeExamples.map((example, i) => (
            <div
              key={i}
              className="bg-slate-800/30 border border-slate-700 rounded-xl overflow-hidden"
            >
              <div className="px-6 py-4 border-b border-slate-700 bg-slate-800/50">
                <h3 className="text-lg font-semibold text-white">
                  {example.title}
                </h3>
              </div>
              <div className="p-6 bg-slate-900/50">
                <pre className="font-mono text-sm text-slate-100 overflow-x-auto whitespace-pre">
                  {example.code.split("\n").map((line, j) => (
                    <div key={j}>
                      {highlightLine(line)}
                      {"\n"}
                    </div>
                  ))}
                </pre>
              </div>
            </div>
          ))}
        </section>
      </main>

      {/* Footer */}
      <footer className="border-t border-slate-800 bg-slate-950/50 mt-12">
        <div className="max-w-7xl mx-auto px-6 py-6 text-center text-sm text-slate-500">
          <p>3AM Language v1.0 • Built for developers who code at 3AM</p>
        </div>
      </footer>
    </div>
  );
}

