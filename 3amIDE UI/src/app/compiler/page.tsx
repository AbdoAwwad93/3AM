"use client";

import type React from "react";
import { useState, useMemo } from "react";
import Link from "next/link";
import {
  Clock,
  ChevronDown,
  ChevronUp,
  CheckCircle,
  Code,
  TreePine,
  Shield,
  Copy,
  Check,
  ArrowLeft,
  AlertCircle,
  Upload,
} from "lucide-react";

interface Token {
  id: number;
  type: string;
  value: string;
}

interface CompilerData {
  fileName: string;
  lexicalAnalysis: {
    tokenCount: number;
    tokens: Token[];
  };
  syntaxAnalysis: {
    ast: string;
    status: "SUCCESS" | "ERROR";
  };
  semanticAnalysis: {
    status: "SUCCESS" | "ERROR";
    message: string;
  };
  overallStatus: "SUCCESS" | "ERROR";
}

function parseCompilerOutput(rawOutput: string): CompilerData | null {
  try {
    const lines = rawOutput.split("\n");

    // Extract filename
    const fileNameMatch = rawOutput.match(/Compiling:\s*(.+\.3am)/i);
    const fileName = fileNameMatch ? fileNameMatch[1].trim() : "unknown.3am";

    // Parse tokens
    const tokens: Token[] = [];
    const tokenRegex = /Token\s+(\d+):\s+(\w+)\s+'([^']*)'/g;
    let tokenMatch;
    while ((tokenMatch = tokenRegex.exec(rawOutput)) !== null) {
      tokens.push({
        id: Number.parseInt(tokenMatch[1]),
        type: tokenMatch[2],
        value: tokenMatch[3],
      });
    }

    // Extract AST
    const astStartIndex = rawOutput.indexOf("=== ABSTRACT SYNTAX TREE ===");
    const astEndIndex = rawOutput.indexOf("--- PHASE: SEMANTIC ANALYSIS ---");
    let ast = "";
    if (astStartIndex !== -1 && astEndIndex !== -1) {
      ast = rawOutput
        .substring(
          astStartIndex + "=== ABSTRACT SYNTAX TREE ===".length,
          astEndIndex
        )
        .trim();
    }

    // Check semantic analysis status
    const semanticSuccess = rawOutput
      .toLowerCase()
      .includes("semantic analysis passed successfully");

    // Check overall status
    const overallSuccess = rawOutput.toLowerCase().includes("status: success");

    return {
      fileName,
      lexicalAnalysis: {
        tokenCount: tokens.length,
        tokens,
      },
      syntaxAnalysis: {
        ast,
        status: ast ? "SUCCESS" : "ERROR",
      },
      semanticAnalysis: {
        status: semanticSuccess ? "SUCCESS" : "ERROR",
        message: semanticSuccess
          ? "Semantic analysis passed successfully."
          : "Semantic analysis failed.",
      },
      overallStatus: overallSuccess ? "SUCCESS" : "ERROR",
    };
  } catch (error) {
    return null;
  }
}

const defaultCompilerOutput = `=== 3AM LANGUAGE COMPILER v1.0 ===
Compiling: input_example.3am


--- PHASE: LEXICAL ANALYSIS (Scanning) ---

Lexical analysis complete. Found 61 tokens.

=== TOKENS ===
Token 0: KEYWORD 'import'
Token 1: STRING 'io'
Token 2: KEYWORD 'schedule'
Token 3: KEYWORD 'startClock'
Token 4: SYMBOL '('
Token 5: SYMBOL ')'
Token 6: SYMBOL '{'
Token 7: KEYWORD 'moment'
Token 8: IDENTIFIER 'name'
Token 9: SYMBOL ';'
Token 10: KEYWORD 'minute'
Token 11: IDENTIFIER 'age'
Token 12: SYMBOL '='
Token 13: NUMBER '4.5'
Token 14: SYMBOL ';'
Token 15: KEYWORD 'tickout'
Token 16: STRING 'Enter your name: '
Token 17: SYMBOL ';'
Token 18: KEYWORD 'tickin'
Token 19: IDENTIFIER 'name'
Token 20: SYMBOL ';'
Token 21: KEYWORD 'tickout'
Token 22: STRING 'Enter your age in seconds: '
Token 23: SYMBOL ';'
Token 24: KEYWORD 'tickin'
Token 25: IDENTIFIER 'age'
Token 26: SYMBOL ';'
Token 27: KEYWORD 'when'
Token 28: SYMBOL '('
Token 29: IDENTIFIER 'age'
Token 30: SYMBOL '>='
Token 31: NUMBER '5'
Token 32: SYMBOL ')'
Token 33: SYMBOL '{'
Token 34: KEYWORD 'tickout'
Token 35: STRING 'your age is bigger'
Token 36: SYMBOL ';'
Token 37: SYMBOL '}'
Token 38: KEYWORD 'otherwise'
Token 39: SYMBOL '{'
Token 40: KEYWORD 'tickout'
Token 41: STRING 'your name is small'
Token 42: SYMBOL '+'
Token 43: NUMBER '5'
Token 44: SYMBOL '+'
Token 45: STRING 'HI'
Token 46: SYMBOL ';'
Token 47: SYMBOL '}'
Token 48: KEYWORD 'tickout'
Token 49: STRING 'Hello, '
Token 50: SYMBOL '+'
Token 51: IDENTIFIER 'name'
Token 52: SYMBOL '+'
Token 53: STRING '! Age seconds = '
Token 54: SYMBOL '+'
Token 55: IDENTIFIER 'age'
Token 56: SYMBOL ';'
Token 57: KEYWORD 'finish'
Token 58: SYMBOL ';'
Token 59: SYMBOL '}'
Token 60: EOF ''


--- PHASE: SYNTAX ANALYSIS (Parsing) ---

Parsing complete. AST generated successfully.

=== ABSTRACT SYNTAX TREE ===
PROGRAM
  IMPORT: io
  STARTCLOCK
    BLOCK
      VARIABLE_DECL: name (type: moment)
      VARIABLE_DECL: age (type: minute)
        NUMBER: 4.5
      TICKOUT
        STRING: Enter your name:
      TICKIN: name
      TICKOUT
        STRING: Enter your age in seconds:
      TICKIN: age
      WHEN
        COMPARISON_OP: >=
          IDENTIFIER: age
          NUMBER: 5
        BLOCK
          TICKOUT
            STRING: your age is bigger
        OTHERWISE
          BLOCK
            TICKOUT
              BINARY_OP: +
                BINARY_OP: +
                  STRING: your name is small
                  NUMBER: 5
                STRING: HI
      TICKOUT
        BINARY_OP: +
          BINARY_OP: +
            BINARY_OP: +
              STRING: Hello,
              IDENTIFIER: name
            STRING: ! Age seconds =
          IDENTIFIER: age
      FINISH

--- PHASE: SEMANTIC ANALYSIS ---


Semantic analysis passed successfully.

=== COMPILATION SUMMARY ===
Status: SUCCESS
All phases completed without errors.`;

function PhaseCard({
  title,
  description,
  icon: Icon,
  iconColor,
  bgColor,
  status,
  children,
  defaultOpen = false,
}: {
  title: string;
  description: string;
  icon: React.ComponentType<{ className?: string }>;
  iconColor: string;
  bgColor: string;
  status: "SUCCESS" | "ERROR";
  children: React.ReactNode;
  defaultOpen?: boolean;
}) {
  const [isOpen, setIsOpen] = useState(defaultOpen);
  const [copied, setCopied] = useState(false);

  const handleCopy = () => {
    const content =
      document.getElementById(`content-${title}`)?.innerText || "";
    navigator.clipboard.writeText(content);
    setCopied(true);
    setTimeout(() => setCopied(false), 2000);
  };

  return (
    <div className="bg-slate-800/30 border border-slate-700 rounded-xl overflow-hidden transition-all duration-300 hover:border-slate-600">
      <button
        onClick={() => setIsOpen(!isOpen)}
        className="w-full px-6 py-5 flex items-center justify-between hover:bg-slate-800/50 transition-colors"
      >
        <div className="flex items-center gap-4">
          <div
            className={`w-12 h-12 rounded-lg ${bgColor} flex items-center justify-center`}
          >
            <Icon className={`w-6 h-6 ${iconColor}`} />
          </div>
          <div className="text-left">
            <h3 className="text-lg font-semibold text-white">{title}</h3>
            <p className="text-sm text-slate-400">{description}</p>
          </div>
        </div>
        <div className="flex items-center gap-3">
          <span
            className={`px-3 py-1 text-xs font-medium rounded-full ${
              status === "SUCCESS"
                ? "bg-green-500/20 text-green-400"
                : "bg-red-500/20 text-red-400"
            }`}
          >
            {status === "SUCCESS" ? "Complete" : "Error"}
          </span>
          {isOpen ? (
            <ChevronUp className="w-5 h-5 text-slate-400" />
          ) : (
            <ChevronDown className="w-5 h-5 text-slate-400" />
          )}
        </div>
      </button>

      {isOpen && (
        <div className="border-t border-slate-700">
          <div className="px-6 py-4 flex justify-end border-b border-slate-700/50">
            <button
              onClick={handleCopy}
              className="flex items-center gap-2 px-3 py-1.5 bg-slate-700 hover:bg-slate-600 text-slate-300 text-sm rounded-lg transition-colors"
            >
              {copied ? (
                <>
                  <Check className="w-4 h-4" />
                  Copied
                </>
              ) : (
                <>
                  <Copy className="w-4 h-4" />
                  Copy Output
                </>
              )}
            </button>
          </div>
          <div
            id={`content-${title}`}
            className="p-6 bg-slate-900/50 max-h-96 overflow-auto"
          >
            {children}
          </div>
        </div>
      )}
    </div>
  );
}

function TokenBadge({ type, value }: { type: string; value: string }) {
  const colorMap: Record<string, string> = {
    KEYWORD: "bg-violet-500/20 text-violet-300 border-violet-500/30",
    STRING: "bg-green-500/20 text-green-300 border-green-500/30",
    IDENTIFIER: "bg-cyan-500/20 text-cyan-300 border-cyan-500/30",
    SYMBOL: "bg-slate-500/20 text-slate-300 border-slate-500/30",
    NUMBER: "bg-orange-500/20 text-orange-300 border-orange-500/30",
    EOF: "bg-red-500/20 text-red-300 border-red-500/30",
  };

  return (
    <span
      className={`inline-flex items-center gap-2 px-3 py-1.5 rounded-lg border text-xs font-mono ${
        colorMap[type] || colorMap.SYMBOL
      }`}
    >
      <span className="text-slate-500">{type}</span>
      <span className="font-semibold">{value || '""'}</span>
    </span>
  );
}

export default function CompilerPage() {
  const [rawOutput, setRawOutput] = useState(defaultCompilerOutput);
  const [showInput, setShowInput] = useState(false);

  const compilerData = useMemo(
    () => parseCompilerOutput(rawOutput),
    [rawOutput]
  );

  const handleFileUpload = (event: React.ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    if (file) {
      const reader = new FileReader();
      reader.onload = (e) => {
        const content = e.target?.result as string;
        setRawOutput(content);
        setShowInput(false);
      };
      reader.readAsText(file);
    }
  };

  if (!compilerData) {
    return (
      <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950 flex items-center justify-center">
        <div className="text-center">
          <AlertCircle className="w-16 h-16 text-red-400 mx-auto mb-4" />
          <h2 className="text-xl font-semibold text-white mb-2">
            Failed to Parse Output
          </h2>
          <p className="text-slate-400 mb-4">
            The compiler output could not be parsed.
          </p>
          <button
            onClick={() => setRawOutput(defaultCompilerOutput)}
            className="px-4 py-2 bg-violet-600 hover:bg-violet-500 text-white rounded-lg transition-colors"
          >
            Load Sample Output
          </button>
        </div>
      </div>
    );
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-950 via-slate-900 to-slate-950">
      {/* Header */}
      <header className="border-b border-slate-800 bg-slate-950/50 backdrop-blur-sm sticky top-0 z-50">
        <div className="max-w-7xl mx-auto px-6 py-4 flex items-center justify-between">
          <div className="flex items-center gap-3">
            <Link href="/" className="flex items-center gap-3">
              <div className="w-10 h-10 rounded-lg bg-gradient-to-br from-violet-500 to-fuchsia-500 flex items-center justify-center">
                <Clock className="w-6 h-6 text-white" />
              </div>
              <div>
                <h1 className="text-xl font-bold text-white">3AM</h1>
                <p className="text-xs text-slate-400">Compiler Output</p>
              </div>
            </Link>
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
          </nav>
        </div>
      </header>

      {/* Main Content */}
      <main className="max-w-5xl mx-auto px-6 py-12">
        <Link
          href="/"
          className="inline-flex items-center gap-2 text-slate-400 hover:text-white transition-colors text-sm mb-8"
        >
          <ArrowLeft className="w-4 h-4" />
          Back to Home
        </Link>

        {/* Title Section */}
        <div className="mb-10">
          <div
            className={`inline-flex items-center gap-2 px-4 py-2 border rounded-full text-sm mb-4 ${
              compilerData.overallStatus === "SUCCESS"
                ? "bg-green-500/10 border-green-500/20 text-green-300"
                : "bg-red-500/10 border-red-500/20 text-red-300"
            }`}
          >
            {compilerData.overallStatus === "SUCCESS" ? (
              <CheckCircle className="w-4 h-4" />
            ) : (
              <AlertCircle className="w-4 h-4" />
            )}
            {compilerData.overallStatus === "SUCCESS"
              ? "Compilation Successful"
              : "Compilation Failed"}
          </div>
          <h1 className="text-4xl font-bold text-white mb-3">
            Compiler Output
          </h1>
          <p className="text-slate-400">
            Compiling:{" "}
            <code className="text-violet-300 bg-slate-800 px-2 py-1 rounded">
              {compilerData.fileName}
            </code>
          </p>
        </div>

        <div className="mb-8">
          <button
            onClick={() => setShowInput(!showInput)}
            className="flex items-center gap-2 px-4 py-2 bg-slate-800 hover:bg-slate-700 text-slate-300 rounded-lg transition-colors text-sm"
          >
            <Upload className="w-4 h-4" />
            {showInput ? "Hide Input" : "Load New Output"}
          </button>

          {showInput && (
            <div className="mt-4 space-y-4">
              <div className="flex items-center gap-4">
                <label className="flex items-center gap-2 px-4 py-2 bg-violet-600 hover:bg-violet-500 text-white rounded-lg transition-colors cursor-pointer text-sm">
                  <Upload className="w-4 h-4" />
                  Upload File
                  <input
                    type="file"
                    accept=".txt"
                    onChange={handleFileUpload}
                    className="hidden"
                  />
                </label>
                <span className="text-slate-500 text-sm">or paste below</span>
              </div>
              <textarea
                value={rawOutput}
                onChange={(e) => setRawOutput(e.target.value)}
                placeholder="Paste your 3AM compiler output here..."
                className="w-full h-64 bg-slate-900 border border-slate-700 rounded-lg p-4 text-slate-300 font-mono text-sm resize-none focus:outline-none focus:border-violet-500 transition-colors"
              />
            </div>
          )}
        </div>

        {/* Phases */}
        <div className="space-y-6">
          {/* Phase 1: Lexical Analysis */}
          <PhaseCard
            title="Phase 1: Lexical Analysis"
            description="Scanning the source code and generating tokens"
            icon={Code}
            iconColor="text-violet-400"
            bgColor="bg-violet-500/20"
            status={
              compilerData.lexicalAnalysis.tokens.length > 0
                ? "SUCCESS"
                : "ERROR"
            }
            defaultOpen={true}
          >
            <div className="space-y-4">
              <div className="flex items-center gap-4 pb-4 border-b border-slate-700">
                <span className="text-slate-400 text-sm">
                  Total tokens found:
                </span>
                <span className="px-3 py-1 bg-violet-500/20 text-violet-300 font-semibold rounded-lg">
                  {compilerData.lexicalAnalysis.tokenCount}
                </span>
              </div>
              <div className="space-y-3">
                <h4 className="text-sm font-semibold text-white mb-3">
                  Token Stream
                </h4>
                <div className="flex flex-wrap gap-2">
                  {compilerData.lexicalAnalysis.tokens.map((token) => (
                    <TokenBadge
                      key={token.id}
                      type={token.type}
                      value={token.value}
                    />
                  ))}
                </div>
              </div>
            </div>
          </PhaseCard>

          {/* Phase 2: Syntax Analysis */}
          <PhaseCard
            title="Phase 2: Syntax Analysis"
            description="Parsing tokens and generating the Abstract Syntax Tree"
            icon={TreePine}
            iconColor="text-fuchsia-400"
            bgColor="bg-fuchsia-500/20"
            status={compilerData.syntaxAnalysis.status}
          >
            <div className="space-y-4">
              <div className="flex items-center gap-4 pb-4 border-b border-slate-700">
                <span className="text-slate-400 text-sm">AST Status:</span>
                <span
                  className={`px-3 py-1 font-semibold rounded-lg ${
                    compilerData.syntaxAnalysis.status === "SUCCESS"
                      ? "bg-green-500/20 text-green-300"
                      : "bg-red-500/20 text-red-300"
                  }`}
                >
                  {compilerData.syntaxAnalysis.status === "SUCCESS"
                    ? "Generated Successfully"
                    : "Failed"}
                </span>
              </div>
              <div>
                <h4 className="text-sm font-semibold text-white mb-3">
                  Abstract Syntax Tree
                </h4>
                <pre className="font-mono text-sm text-slate-200 whitespace-pre leading-relaxed bg-slate-800/50 p-4 rounded-lg overflow-x-auto">
                  {compilerData.syntaxAnalysis.ast
                    .split("\n")
                    .map((line, i) => {
                      const indent = line.search(/\S/);
                      const content = line.trim();
                      if (!content) return null;
                      const isNode = content.includes(":");
                      const [nodeName, nodeValue] = isNode
                        ? content.split(":")
                        : [content, ""];

                      return (
                        <div
                          key={i}
                          style={{
                            paddingLeft: `${Math.max(0, indent) * 8}px`,
                          }}
                        >
                          {isNode ? (
                            <>
                              <span className="text-fuchsia-400">
                                {nodeName}
                              </span>
                              <span className="text-slate-400">:</span>
                              <span className="text-cyan-300">{nodeValue}</span>
                            </>
                          ) : (
                            <span className="text-yellow-300">{content}</span>
                          )}
                        </div>
                      );
                    })}
                </pre>
              </div>
            </div>
          </PhaseCard>

          {/* Phase 3: Semantic Analysis */}
          <PhaseCard
            title="Phase 3: Semantic Analysis"
            description="Type checking and semantic validation"
            icon={Shield}
            iconColor="text-cyan-400"
            bgColor="bg-cyan-500/20"
            status={compilerData.semanticAnalysis.status}
          >
            <div className="space-y-4">
              <div className="flex items-center justify-center py-8">
                <div className="text-center">
                  <div
                    className={`w-16 h-16 rounded-full flex items-center justify-center mx-auto mb-4 ${
                      compilerData.semanticAnalysis.status === "SUCCESS"
                        ? "bg-green-500/20"
                        : "bg-red-500/20"
                    }`}
                  >
                    {compilerData.semanticAnalysis.status === "SUCCESS" ? (
                      <CheckCircle className="w-8 h-8 text-green-400" />
                    ) : (
                      <AlertCircle className="w-8 h-8 text-red-400" />
                    )}
                  </div>
                  <h4 className="text-xl font-semibold text-white mb-2">
                    {compilerData.semanticAnalysis.message}
                  </h4>
                  <p className="text-slate-400 text-sm">
                    {compilerData.semanticAnalysis.status === "SUCCESS"
                      ? "All type checks and semantic validations passed without errors."
                      : "Some semantic validations failed. Please check your code."}
                  </p>
                </div>
              </div>
              <div className="bg-slate-800/50 rounded-lg p-4 border border-slate-700">
                <h5 className="text-sm font-semibold text-white mb-2">
                  Checks Performed
                </h5>
                <ul className="space-y-2 text-sm text-slate-400">
                  <li className="flex items-center gap-2">
                    <CheckCircle className="w-4 h-4 text-green-400" />
                    Variable declarations validated
                  </li>
                  <li className="flex items-center gap-2">
                    <CheckCircle className="w-4 h-4 text-green-400" />
                    Type compatibility verified
                  </li>
                  <li className="flex items-center gap-2">
                    <CheckCircle className="w-4 h-4 text-green-400" />
                    Function signatures checked
                  </li>
                  <li className="flex items-center gap-2">
                    <CheckCircle className="w-4 h-4 text-green-400" />
                    Control flow analysis complete
                  </li>
                </ul>
              </div>
            </div>
          </PhaseCard>
        </div>

        {/* Summary Card */}
        <div
          className={`mt-10 border rounded-xl p-6 ${
            compilerData.overallStatus === "SUCCESS"
              ? "bg-gradient-to-r from-green-500/10 to-cyan-500/10 border-green-500/20"
              : "bg-gradient-to-r from-red-500/10 to-orange-500/10 border-red-500/20"
          }`}
        >
          <div className="flex items-center gap-4">
            <div
              className={`w-12 h-12 rounded-full flex items-center justify-center ${
                compilerData.overallStatus === "SUCCESS"
                  ? "bg-green-500/20"
                  : "bg-red-500/20"
              }`}
            >
              {compilerData.overallStatus === "SUCCESS" ? (
                <CheckCircle className="w-6 h-6 text-green-400" />
              ) : (
                <AlertCircle className="w-6 h-6 text-red-400" />
              )}
            </div>
            <div>
              <h3 className="text-lg font-semibold text-white">
                Compilation Summary
              </h3>
              <p className="text-slate-400 text-sm">
                Status: {compilerData.overallStatus} —{" "}
                {compilerData.overallStatus === "SUCCESS"
                  ? "All phases completed without errors."
                  : "Some phases encountered errors."}
              </p>
            </div>
          </div>
        </div>
      </main>

      {/* Footer */}
      <footer className="border-t border-slate-800 bg-slate-950/50 mt-12">
        <div className="max-w-7xl mx-auto px-6 py-6 text-center text-sm text-slate-500">
          <p>3AM Language Compiler v1.0</p>
        </div>
      </footer>
    </div>
  );
}
