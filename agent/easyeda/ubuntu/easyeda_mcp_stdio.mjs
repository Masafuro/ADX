#!/usr/bin/env node
/**
 * Zero-dependency MCP stdio-to-HTTP bridge for EasyEDA Pro over SSH reverse tunnel.
 *
 * Designed for on-demand / non-resident Windows bridge operation:
 * - Reads Bearer token dynamically from EASYEDA_MCP_TOKEN / MCP_TOKEN or ~/.config/easyeda-bridge/token.
 * - If the Windows bridge is offline when the MCP client starts, still serves initialize & tools/list
 *   so the agent does not need an MCP reload after launching start_easyeda_bridge.bat on Windows.
 * - Forwards live JSON-RPC calls to http://127.0.0.1:3030/mcp as soon as the tunnel is up.
 */

import fs from "node:fs";
import os from "node:os";
import path from "node:path";
import readline from "node:readline";

const MCP_URL = process.env.EASYEDA_MCP_URL || "http://127.0.0.1:3030/mcp";
const TOKEN_FILE =
  process.env.EASYEDA_MCP_TOKEN_FILE ||
  path.join(os.homedir(), ".config", "easyeda-bridge", "token");
const CALL_TIMEOUT_MS = Number(process.env.EASYEDA_MCP_TIMEOUT_MS || 180000);
const PROBE_TIMEOUT_MS = Number(process.env.EASYEDA_MCP_PROBE_TIMEOUT_MS || 2000);

const FALLBACK_TOOLS = [
  {
    name: "easyeda-pro.help",
    description:
      "ALWAYS call this FIRST when interacting with a CLI capability. Returns available subcommands, their purpose, input schemas, and usage examples. After understanding via mcli.help, use mcli.discover to find specific commands or mcli.call to execute them.",
    inputSchema: {
      type: "object",
      properties: {
        path: {
          type: "string",
          description:
            "Optional command path like 'session' or 'doc.api'. Omit to see top-level.",
        },
      },
    },
  },
  {
    name: "easyeda-pro.discover",
    description:
      "Search for commands by keyword AFTER using mcli.help to understand the tool. Returns matching command paths and summaries.",
    inputSchema: {
      type: "object",
      properties: {
        query: { type: "string", description: "Search query" },
        limit: { type: "number", description: "Max results (default 10)" },
      },
      required: ["query"],
    },
  },
  {
    name: "easyeda-pro.call",
    description:
      "Execute a registered EasyEDA Pro CLI command (e.g. doctor, open, session.list, invoke, doc.api). Call help first for schemas.",
    inputSchema: {
      type: "object",
      properties: {
        path: {
          type: "string",
          description: "Command path like 'doctor', 'open', 'session.list', 'invoke', 'doc.api'",
        },
        input: {
          type: "object",
          additionalProperties: true,
          description: "Command input parameters",
        },
      },
      required: ["path"],
    },
  },
];

function readToken() {
  const envTok = process.env.EASYEDA_MCP_TOKEN || process.env.MCP_TOKEN;
  if (envTok && envTok.trim()) return envTok.trim();
  try {
    if (fs.existsSync(TOKEN_FILE)) {
      return fs.readFileSync(TOKEN_FILE, "utf8").trim();
    }
  } catch {
    // Ignore read error; reported when calling upstream
  }
  return "";
}

process.stdout.on("error", (err) => {
  if (err && err.code === "EPIPE") process.exit(0);
});

function send(obj) {
  process.stdout.write(JSON.stringify(obj) + "\n");
}

async function forwardHttp(msg, timeoutMs) {
  const token = readToken();
  if (!token) {
    return {
      ok: false,
      reason: `Bearer token not found. Set EASYEDA_MCP_TOKEN or create ${TOKEN_FILE}.`,
    };
  }

  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), timeoutMs);
  try {
    const res = await fetch(MCP_URL, {
      method: "POST",
      headers: {
        Authorization: `Bearer ${token}`,
        "Content-Type": "application/json",
        Accept: "application/json, text/event-stream",
      },
      body: JSON.stringify(msg),
      signal: controller.signal,
    });

    const text = await res.text();
    if (!res.ok) {
      return {
        ok: false,
        status: res.status,
        reason: `EasyEDA MCP HTTP ${res.status}: ${text.slice(0, 400)}`,
      };
    }

    // Handle plain JSON or SSE ("data: {...}")
    let payload = text.trim();
    if (payload.startsWith("event:") || payload.startsWith("data:")) {
      const dataLines = payload
        .split(/\r?\n/)
        .filter((l) => l.startsWith("data:"))
        .map((l) => l.slice(5).trim());
      if (dataLines.length > 0) payload = dataLines[dataLines.length - 1];
    }

    const parsed = JSON.parse(payload);
    return { ok: true, data: parsed };
  } catch (err) {
    const detail = err?.name === "AbortError" ? `timeout (${timeoutMs}ms)` : String(err?.cause?.code || err?.message || err);
    return {
      ok: false,
      reason: `Cannot reach EasyEDA Pro bridge at ${MCP_URL} (${detail}). Start start_easyeda_bridge.bat on Windows and retry.`,
    };
  } finally {
    clearTimeout(timer);
  }
}

async function handleMessage(msg) {
  // Notifications have no id -> no response per JSON-RPC 2.0
  if (msg.id === undefined || msg.id === null) {
    return;
  }

  const { id, method } = msg;

  if (method === "ping") {
    send({ jsonrpc: "2.0", id, result: {} });
    return;
  }

  if (method === "initialize") {
    const up = await forwardHttp(msg, PROBE_TIMEOUT_MS);
    if (up.ok && up.data) {
      send({ ...up.data, id });
      return;
    }
    send({
      jsonrpc: "2.0",
      id,
      result: {
        protocolVersion: msg.params?.protocolVersion || "2025-06-18",
        capabilities: { tools: {} },
        serverInfo: {
          name: "EasyEDA Pro (bridge standby)",
          version: "4.1.105",
        },
      },
    });
    return;
  }

  if (method === "tools/list") {
    const up = await forwardHttp(msg, PROBE_TIMEOUT_MS);
    if (up.ok && up.data) {
      send({ ...up.data, id });
      return;
    }
    send({
      jsonrpc: "2.0",
      id,
      result: { tools: FALLBACK_TOOLS },
    });
    return;
  }

  if (method === "tools/call") {
    const up = await forwardHttp(msg, CALL_TIMEOUT_MS);
    if (up.ok && up.data) {
      send({ ...up.data, id });
      return;
    }
    send({
      jsonrpc: "2.0",
      id,
      result: {
        isError: true,
        content: [{ type: "text", text: up.reason || "Unknown bridge error" }],
      },
    });
    return;
  }

  // Forward any other JSON-RPC method cleanly
  const up = await forwardHttp(msg, PROBE_TIMEOUT_MS);
  if (up.ok && up.data) {
    send({ ...up.data, id });
    return;
  }
  send({
    jsonrpc: "2.0",
    id,
    error: { code: -32601, message: up.reason || `Method not available: ${method}` },
  });
}

const rl = readline.createInterface({
  input: process.stdin,
  crlfDelay: Infinity,
});

// Process sequentially or safely without interleaving stdout writes
rl.on("line", (line) => {
  const trimmed = line.trim();
  if (!trimmed) return;
  let msg;
  try {
    msg = JSON.parse(trimmed);
  } catch (err) {
    send({
      jsonrpc: "2.0",
      id: null,
      error: { code: -32700, message: `Parse error: ${err.message}` },
    });
    return;
  }
  handleMessage(msg).catch((err) => {
    if (msg && msg.id !== undefined && msg.id !== null) {
      send({
        jsonrpc: "2.0",
        id: msg.id,
        error: { code: -32603, message: `Internal proxy error: ${err?.message || err}` },
      });
    }
  });
});
