#!/usr/bin/env node
/**
 * Small CLI helper to invoke EasyEDA Pro MCP tools over http://127.0.0.1:3030/mcp
 * using ~/.config/easyeda-bridge/token.
 *
 * Usage:
 *   node mcp_call.mjs <command-path> [json-input]
 *   node mcp_call.mjs invoke --session <id> --file <script.js> [--args '<json>'] [--timeout 60000]
 */

import fs from "node:fs";
import os from "node:os";
import path from "node:path";

const MCP_URL = process.env.EASYEDA_MCP_URL || "http://127.0.0.1:3030/mcp";
const TOKEN_FILE =
  process.env.EASYEDA_MCP_TOKEN_FILE ||
  path.join(os.homedir(), ".config", "easyeda-bridge", "token");

function readToken() {
  const envTok = process.env.EASYEDA_MCP_TOKEN || process.env.MCP_TOKEN;
  if (envTok && envTok.trim()) return envTok.trim();
  return fs.readFileSync(TOKEN_FILE, "utf8").trim();
}

async function callMcp(cmdPath, inputObj, timeoutMs = 120000) {
  const token = readToken();
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
      body: JSON.stringify({
        jsonrpc: "2.0",
        id: 1,
        method: "tools/call",
        params: {
          name: "easyeda-pro.call",
          arguments: { path: cmdPath, input: inputObj },
        },
      }),
      signal: controller.signal,
    });
    const text = await res.text();
    if (!res.ok) {
      throw new Error(`HTTP ${res.status}: ${text}`);
    }
    const rpc = JSON.parse(text);
    const contentText = rpc?.result?.content?.[0]?.text ?? JSON.stringify(rpc);
    if (rpc?.result?.isError) {
      console.error(contentText);
      process.exit(1);
    }
    process.stdout.write(contentText + "\n");
  } finally {
    clearTimeout(timer);
  }
}

const args = process.argv.slice(2);
if (args.length === 0) {
  console.error("Usage: node mcp_call.mjs <path> [json-input] | invoke --session <id> --file <js> [--args <json>]");
  process.exit(1);
}

const cmdPath = args[0];
if (cmdPath === "invoke" && args.includes("--file")) {
  const input = { extUuid: "eda" };
  let timeoutMs = 120000;
  for (let i = 1; i < args.length; i++) {
    if (args[i] === "--session") input.session = args[++i];
    else if (args[i] === "--file") input.code = fs.readFileSync(args[++i], "utf8");
    else if (args[i] === "--args") input.args = args[++i];
    else if (args[i] === "--timeout") {
      input.timeout = Number(args[++i]);
      timeoutMs = Math.max(timeoutMs, input.timeout + 15000);
    }
  }
  await callMcp("invoke", input, timeoutMs);
} else {
  const inputObj = args[1] ? JSON.parse(args[1]) : {};
  await callMcp(cmdPath, inputObj);
}
