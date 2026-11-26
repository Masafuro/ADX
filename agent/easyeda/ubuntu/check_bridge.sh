#!/usr/bin/env bash
# Ubuntu-side step-by-step diagnostic script for EasyEDA Pro HTTP MCP over SSH reverse tunnel.
# Usage:
#   MCP_TOKEN="<your-token>" ./check_bridge.sh [port] [path]
# Example:
#   MCP_TOKEN="abc123..." ./check_bridge.sh 3030

set -u

PORT="${1:-3030}"
ENDPOINT_PATH="${2:-/mcp}"
BASE_URL="http://127.0.0.1:${PORT}"

if [ -z "${MCP_TOKEN:-}" ] && [ -r "${HOME}/.config/easyeda-bridge/token" ]; then
  MCP_TOKEN="$(tr -d '\r\n' < "${HOME}/.config/easyeda-bridge/token")"
fi

S_PORT="FAIL"
S_UNAUTH="SKIP"
S_INIT="SKIP"
S_TOOLS="SKIP"
ACTIVE_PATH=""

TMP_DIR="$(mktemp -d /tmp/easyeda_bridge_check.XXXXXX)"
trap 'rm -rf "${TMP_DIR}"' EXIT

echo "================================================================="
echo " EasyEDA Pro Bridge Diagnostic (Ubuntu / LXD side)"
echo " Target: ${BASE_URL} (default path: ${ENDPOINT_PATH})"
echo "================================================================="

# ------------------------------------------------------------------
# [1/4] Local TCP port check (SSH reverse tunnel)
# ------------------------------------------------------------------
echo ""
echo "[1/4] Checking if SSH reverse tunnel is listening on 127.0.0.1:${PORT} ..."
if command -v ss >/dev/null 2>&1; then
  SS_OUT="$(ss -tlnH "sport = :${PORT}" 2>/dev/null || true)"
  if [ -n "${SS_OUT}" ]; then
    echo "      ss -tln: ${SS_OUT}"
  else
    echo "      ss -tln: (no listener on port ${PORT})"
  fi
fi

if (echo >/dev/tcp/127.0.0.1/"${PORT}") 2>/dev/null; then
  echo "      [OK] TCP connection to 127.0.0.1:${PORT} succeeded."
  S_PORT="PASS"
else
  echo "      [FAIL] Cannot connect to 127.0.0.1:${PORT}."
  echo "      Hint: Check Windows terminal [5/5] for 'remote forward success for: listen 127.0.0.1:${PORT}'."
  echo ""
  echo "=== SUMMARY ==="
  echo "  [1/4] Tunnel port  : FAIL"
  echo "  [2/4] Auth reject  : SKIP"
  echo "  [3/4] Initialize   : SKIP"
  echo "  [4/4] tools/list   : SKIP"
  exit 1
fi

INIT_PAYLOAD='{"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-03-26","capabilities":{},"clientInfo":{"name":"adx-bridge-check","version":"0.1.0"}}}'

# ------------------------------------------------------------------
# [2/4] Unauthenticated probe (should be rejected if --token is set)
# ------------------------------------------------------------------
echo ""
echo "[2/4] Sending POST ${ENDPOINT_PATH} WITHOUT Bearer token (expecting 401/403)..."
CURL_ERR=""
UNAUTH_CODE="$(curl -sS --max-time 5 \
  -D "${TMP_DIR}/unauth.hdr" \
  -o "${TMP_DIR}/unauth.body" \
  -w "%{http_code}" \
  -X POST "${BASE_URL}${ENDPOINT_PATH}" \
  -H "Content-Type: application/json" \
  -H "Accept: application/json, text/event-stream" \
  -d "${INIT_PAYLOAD}" 2>"${TMP_DIR}/unauth.err" || true)"
CURL_ERR="$(cat "${TMP_DIR}/unauth.err" 2>/dev/null || true)"

echo "      HTTP status: ${UNAUTH_CODE}"
if [ -n "${CURL_ERR}" ]; then
  echo "      curl stderr: ${CURL_ERR}"
fi
if [ -s "${TMP_DIR}/unauth.body" ]; then
  echo "      body: $(head -c 300 "${TMP_DIR}/unauth.body")"
fi

if [ "${UNAUTH_CODE}" = "401" ] || [ "${UNAUTH_CODE}" = "403" ]; then
  echo "      [OK] Unauthenticated request properly rejected (${UNAUTH_CODE})."
  S_UNAUTH="PASS"
elif [ "${UNAUTH_CODE}" = "000" ]; then
  echo "      [FAIL] HTTP failed (status 000). Tunnel exists, but Windows 127.0.0.1:${PORT} did not respond."
  S_UNAUTH="FAIL"
else
  echo "      [WARN] Unexpected HTTP status ${UNAUTH_CODE} (expected 401/403)."
  S_UNAUTH="WARN(${UNAUTH_CODE})"
fi

# ------------------------------------------------------------------
# [3/4] Authenticated MCP initialize
# ------------------------------------------------------------------
echo ""
if [ -z "${MCP_TOKEN:-}" ]; then
  echo "[3/4] SKIP: MCP_TOKEN environment variable is not set."
  echo "      Run with: MCP_TOKEN='<your-token>' $0 ${PORT}"
else
  echo "[3/4] Sending authenticated MCP 'initialize'..."
  for try_path in "${ENDPOINT_PATH}" "/"; do
    echo "      --- Trying POST ${BASE_URL}${try_path} ---"
    HTTP_CODE="$(curl -sS --max-time 8 \
      -D "${TMP_DIR}/init.hdr" \
      -o "${TMP_DIR}/init.body" \
      -w "%{http_code}" \
      -X POST "${BASE_URL}${try_path}" \
      -H "Authorization: Bearer ${MCP_TOKEN}" \
      -H "Content-Type: application/json" \
      -H "Accept: application/json, text/event-stream" \
      -d "${INIT_PAYLOAD}" 2>"${TMP_DIR}/init.err" || true)"
    INIT_ERR="$(cat "${TMP_DIR}/init.err" 2>/dev/null || true)"
    echo "      HTTP status: ${HTTP_CODE}"
    if [ -n "${INIT_ERR}" ]; then
      echo "      curl stderr: ${INIT_ERR}"
    fi
    if [ -s "${TMP_DIR}/init.hdr" ]; then
      echo "      headers:"
      sed 's/^/        /' "${TMP_DIR}/init.hdr"
    fi
    if [ -s "${TMP_DIR}/init.body" ]; then
      echo "      body:"
      head -c 1000 "${TMP_DIR}/init.body" | sed 's/^/        /'
      echo ""
    fi
    if [ "${HTTP_CODE}" = "200" ]; then
      ACTIVE_PATH="${try_path}"
      S_INIT="PASS (${try_path})"
      echo "      [OK] MCP initialize succeeded on '${try_path}'."
      break
    fi
  done
  if [ -z "${ACTIVE_PATH}" ]; then
    S_INIT="FAIL"
    echo "      [FAIL] MCP initialize did not return HTTP 200."
  fi
fi

# ------------------------------------------------------------------
# [4/4] Authenticated MCP tools/list
# ------------------------------------------------------------------
echo ""
if [ -n "${ACTIVE_PATH}" ]; then
  echo "[4/4] Sending authenticated MCP 'tools/list' to ${BASE_URL}${ACTIVE_PATH} ..."
  TOOLS_PAYLOAD='{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}'
  TOOLS_CODE="$(curl -sS --max-time 8 \
    -D "${TMP_DIR}/tools.hdr" \
    -o "${TMP_DIR}/tools.body" \
    -w "%{http_code}" \
    -X POST "${BASE_URL}${ACTIVE_PATH}" \
    -H "Authorization: Bearer ${MCP_TOKEN}" \
    -H "Content-Type: application/json" \
    -H "Accept: application/json, text/event-stream" \
    -d "${TOOLS_PAYLOAD}" 2>"${TMP_DIR}/tools.err" || true)"
  TOOLS_ERR="$(cat "${TMP_DIR}/tools.err" 2>/dev/null || true)"
  echo "      HTTP status: ${TOOLS_CODE}"
  if [ -n "${TOOLS_ERR}" ]; then
    echo "      curl stderr: ${TOOLS_ERR}"
  fi
  if [ "${TOOLS_CODE}" = "200" ]; then
    S_TOOLS="PASS"
    if command -v python3 >/dev/null 2>&1; then
      python3 - "${TMP_DIR}/tools.body" <<'PY' || true
import json, sys
raw = open(sys.argv[1], "r", encoding="utf-8", errors="replace").read().strip()
# Support both plain JSON and SSE ("data: {...}") responses
lines = [l[5:].strip() for l in raw.splitlines() if l.startswith("data:")]
payload = lines[-1] if lines else raw
try:
    obj = json.loads(payload)
    tools = obj.get("result", {}).get("tools", [])
    print(f"      [OK] Discovered {len(tools)} MCP tools:")
    for t in tools:
        desc = (t.get("description") or "").splitlines()[0][:70]
        print(f"        - {t.get('name')}: {desc}")
except Exception as e:
    print(f"      [INFO] Raw response ({e}):")
    print("        " + raw[:1200])
PY
    else
      head -c 1500 "${TMP_DIR}/tools.body"
      echo ""
    fi
  else
    S_TOOLS="FAIL (HTTP ${TOOLS_CODE})"
    head -c 800 "${TMP_DIR}/tools.body" 2>/dev/null || true
    echo ""
  fi
else
  echo "[4/4] SKIP: tools/list skipped because initialize did not succeed."
fi

echo ""
echo "================================================================="
echo " SUMMARY"
echo "   [1/4] Tunnel port  : ${S_PORT}"
echo "   [2/4] Auth reject  : ${S_UNAUTH}"
echo "   [3/4] Initialize   : ${S_INIT}"
echo "   [4/4] tools/list   : ${S_TOOLS}"
echo "================================================================="
