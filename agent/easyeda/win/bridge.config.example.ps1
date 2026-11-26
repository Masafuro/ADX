# Copy this file to "bridge.config.local.ps1" in the same folder and edit values.
# "bridge.config.local.ps1" is git-ignored so tokens and IPs are never committed.

# Ubuntu / LXD SSH target:
# - If using an entry from ~/.ssh/config (including custom Port / ProxyJump / User / IdentityFile),
#   set $UBUNTU_HOST to the Host alias and leave $UBUNTU_USER, $SSH_PORT, $SSH_KEY empty.
# - Otherwise set $UBUNTU_USER, $UBUNTU_HOST, and optionally $SSH_PORT / $SSH_KEY.
$UBUNTU_USER = "ubuntu"
$UBUNTU_HOST = "192.168.1.100"
$SSH_PORT    = ""                # Leave empty to use ~/.ssh/config Port (or default 22)

# Optional SSH private key path (leave empty to use ~/.ssh/config / default ~/.ssh/id_* / ssh-agent)
$SSH_KEY     = ""

# Port used on both Windows (127.0.0.1) and Ubuntu (127.0.0.1) via SSH reverse tunnel
$MCP_PORT    = 3030

# Bearer token for EasyEDA Pro HTTP MCP auth.
# Generate a random token in PowerShell with:
#   -join ((1..32) | ForEach-Object { '{0:x}' -f (Get-Random -Maximum 16) })
$MCP_TOKEN   = "REPLACE_WITH_RANDOM_TOKEN"
