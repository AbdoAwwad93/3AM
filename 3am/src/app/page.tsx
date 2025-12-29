import Link from "next/link";
import { Clock, Code, FileText, Zap, ArrowRight } from "lucide-react";

export default function HomePage() {
  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950">
      {/* Header */}
      <header className="border-b border-slate-800 bg-slate-950/50 backdrop-blur-sm sticky top-0 z-50">
        <div className="max-w-7xl mx-auto px-6 py-4 flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className="w-10 h-10 rounded-lg bg-gradient-to-br from-violet-500 to-fuchsia-500 flex items-center justify-center">
              <Clock className="w-6 h-6 text-white" />
            </div>
            <div>
              <h1 className="text-xl font-bold text-white">3AM</h1>
              <p className="text-xs text-slate-400">Programming Language</p>
            </div>
          </div>
          <nav className="flex items-center gap-6">
            <Link
              href="/about"
              className="text-slate-400 hover:text-white transition-colors text-sm"
            >
              About
            </Link>
            <Link
              href="/editor"
              className="text-slate-400 hover:text-white transition-colors text-sm"
            >
              Editor
            </Link>
            <Link
              href="/compiler"
              className="text-slate-400 hover:text-white transition-colors text-sm"
            >
              Compiler Output
            </Link>
          </nav>
        </div>
      </header>

      {/* Hero Section */}
      <main className="max-w-7xl mx-auto px-6 py-20">
        <div className="text-center mb-16">
          <div className="inline-flex items-center gap-2 px-4 py-2 bg-violet-500/10 border border-violet-500/20 rounded-full text-violet-300 text-sm mb-6">
            <Zap className="w-4 h-4" />
            Version 1.0 Released
          </div>
          <h1 className="text-5xl md:text-7xl font-bold text-white mb-6 text-balance">
            Code at{" "}
            <span className="text-transparent bg-clip-text bg-gradient-to-r from-violet-400 to-fuchsia-400">
              3AM
            </span>
          </h1>
          <p className="text-xl text-slate-400 max-w-2xl mx-auto mb-10 text-pretty">
            A time-themed programming language with intuitive syntax. Write code
            that flows like the ticking of a clock.
          </p>
          <div className="flex items-center justify-center gap-4">
            <Link
              href="/editor"
              className="flex items-center gap-2 bg-gradient-to-r from-violet-600 to-fuchsia-600 hover:from-violet-700 hover:to-fuchsia-700 text-white font-semibold py-3 px-8 rounded-lg transition-all duration-200 shadow-lg hover:shadow-xl"
            >
              Try the Editor
              <ArrowRight className="w-4 h-4" />
            </Link>
            <Link
              href="/about"
              className="flex items-center gap-2 bg-slate-800 hover:bg-slate-700 text-slate-300 font-semibold py-3 px-8 rounded-lg transition-colors"
            >
              Learn More
            </Link>
          </div>
        </div>

        {/* Features Grid */}
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-20">
          <div className="bg-slate-800/30 border border-slate-700 rounded-xl p-6 hover:border-violet-500/50 transition-colors">
            <div className="w-12 h-12 rounded-lg bg-violet-500/20 flex items-center justify-center mb-4">
              <Clock className="w-6 h-6 text-violet-400" />
            </div>
            <h3 className="text-lg font-semibold text-white mb-2">
              Time-Themed Syntax
            </h3>
            <p className="text-slate-400 text-sm">
              Use intuitive keywords like{" "}
              <code className="text-violet-300">schedule</code>,{" "}
              <code className="text-violet-300">tickout</code>, and{" "}
              <code className="text-violet-300">moment</code>.
            </p>
          </div>
          <div className="bg-slate-800/30 border border-slate-700 rounded-xl p-6 hover:border-fuchsia-500/50 transition-colors">
            <div className="w-12 h-12 rounded-lg bg-fuchsia-500/20 flex items-center justify-center mb-4">
              <Code className="w-6 h-6 text-fuchsia-400" />
            </div>
            <h3 className="text-lg font-semibold text-white mb-2">
              Multi-Phase Compiler
            </h3>
            <p className="text-slate-400 text-sm">
              Full lexical analysis, syntax parsing, and semantic analysis with
              detailed output.
            </p>
          </div>
          <div className="bg-slate-800/30 border border-slate-700 rounded-xl p-6 hover:border-cyan-500/50 transition-colors">
            <div className="w-12 h-12 rounded-lg bg-cyan-500/20 flex items-center justify-center mb-4">
              <FileText className="w-6 h-6 text-cyan-400" />
            </div>
            <h3 className="text-lg font-semibold text-white mb-2">
              Clear Documentation
            </h3>
            <p className="text-slate-400 text-sm">
              Comprehensive guides and examples to get you started coding in
              3AM.
            </p>
          </div>
        </div>

        {/* Code Preview */}
        <div className="bg-slate-800/30 border border-slate-700 rounded-xl p-8">
          <h2 className="text-2xl font-bold text-white mb-6">
            Hello World in 3AM
          </h2>
          <div className="bg-slate-900 rounded-lg p-6 font-mono text-sm">
            <pre className="text-slate-100">
              <span className="text-fuchsia-400">#startclock</span>{" "}
              <span className="text-cyan-400">timeline</span>
              {"\n\n"}
              <span className="text-violet-400">schedule</span>{" "}
              <span className="text-yellow-300">start</span>() {"{"}
              {"\n"}
              {"  "}
              <span className="text-violet-400">tickout</span>{" "}
              <span className="text-green-400">
                "Clock started — Welcome to 3AM!"
              </span>
              ;{"\n"}
              {"  "}
              <span className="text-violet-400">finish</span>{" "}
              <span className="text-orange-400">0</span>;{"\n"}
              {"}"}
            </pre>
          </div>
        </div>
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
