#!/usr/bin/env python3
"""
ADX Hackaday.io Weekly DevLog Compiler
Copyright (c) 2026 ADX Project Contributors
SPDX-License-Identifier: CC-BY-4.0

This script compiles weekly markdown logs in docs/blog/posts/
into optimized, paste-ready HTML snippets for Hackaday.io.
It also optimizes images placed in docs/blog/raw_assets/ into docs/blog/images/.
"""

import sys
import os
import re
import argparse
from pathlib import Path

# Optional imports with graceful handling
try:
    from PIL import Image, ImageOps
    HAS_PIL = True
except ImportError:
    HAS_PIL = False

try:
    import markdown
    HAS_MARKDOWN = True
except ImportError:
    HAS_MARKDOWN = False


# Base URLs & Constants
CDN_PROVIDERS = {
    "jsdelivr": "https://cdn.jsdelivr.net/gh/Masafuro/ADX@main/docs/blog/images",
    "raw": "https://raw.githubusercontent.com/Masafuro/ADX/refs/heads/main/docs/blog/images",
    "pages": "https://masafuro.github.io/ADX/blog/images",
}
DEFAULT_CDN_PROVIDER = "jsdelivr"

MAX_IMAGE_WIDTH = 1200
JPEG_QUALITY = 85
HERO_ASPECT_RATIO = 16.0 / 9.0


def get_base_dirs():
    """Resolve key directory paths relative to this script."""
    script_dir = Path(__file__).resolve().parent
    blog_dir = script_dir.parent
    return {
        "blog": blog_dir,
        "raw_assets": blog_dir / "raw_assets",
        "images": blog_dir / "images",
        "posts": blog_dir / "posts",
        "dist": blog_dir / "dist",
    }


def optimize_image(src_path: Path, dest_dir: Path, is_hero: bool = False) -> str:
    """
    Optimize an image from raw_assets into images/:
    - Correct EXIF orientation
    - Auto-crop Hero images to 16:9
    - Resize if width exceeds MAX_IMAGE_WIDTH
    - Convert to progressive JPEG or optimized PNG
    """
    if not HAS_PIL:
        # Fallback if Pillow is not available: copy file directly
        dest_path = dest_dir / src_path.name
        dest_path.write_bytes(src_path.read_bytes())
        return src_path.name

    with Image.open(src_path) as img:
        # Transpose based on EXIF orientation (handles phone cameras)
        img = ImageOps.exif_transpose(img)

        # Handle Hero image aspect ratio (16:9)
        if is_hero:
            target_width = img.width
            target_height = int(target_width / HERO_ASPECT_RATIO)
            if target_height > img.height:
                target_height = img.height
                target_width = int(target_height * HERO_ASPECT_RATIO)
            
            # Center crop to 16:9
            left = (img.width - target_width) // 2
            top = (img.height - target_height) // 2
            right = left + target_width
            bottom = top + target_height
            img = img.crop((left, top, right, bottom))

        # Resize if width exceeds MAX_IMAGE_WIDTH
        if img.width > MAX_IMAGE_WIDTH:
            new_height = int(img.height * (MAX_IMAGE_WIDTH / img.width))
            resample_filter = getattr(Image, "Resampling", Image).LANCZOS
            img = img.resize((MAX_IMAGE_WIDTH, new_height), resample=resample_filter)

        # Determine target extension and format
        ext = src_path.suffix.lower()
        if ext in [".png", ".webp"] and img.mode in ("RGBA", "LA", "P"):
            dest_filename = f"{src_path.stem}.png"
            dest_path = dest_dir / dest_filename
            img.save(dest_path, format="PNG", optimize=True)
        else:
            # Convert RGBA to RGB for JPEG compatibility
            if img.mode != "RGB":
                img = img.convert("RGB")
            dest_filename = f"{src_path.stem}.jpg"
            dest_path = dest_dir / dest_filename
            img.save(dest_path, format="JPEG", quality=JPEG_QUALITY, progressive=True, optimize=True)

        return dest_filename


def process_all_raw_images(dirs: dict) -> dict:
    """Process all raw images in raw_assets and return a mapping of filename -> optimized filename."""
    raw_dir = dirs["raw_assets"]
    dest_dir = dirs["images"]
    dest_dir.mkdir(parents=True, exist_ok=True)

    mapping = {}
    valid_exts = {".jpg", ".jpeg", ".png", ".webp", ".bmp", ".heic"}

    for file in raw_dir.iterdir():
        if file.is_file() and file.suffix.lower() in valid_exts:
            is_hero = "hero" in file.stem.lower()
            optimized_name = optimize_image(file, dest_dir, is_hero=is_hero)
            mapping[file.name] = optimized_name
            mapping[file.stem] = optimized_name  # also map without ext
            print(f"  [Image] Optimized: {file.name} -> images/{optimized_name}")

    return mapping


def parse_frontmatter(content: str):
    """Simple parser for YAML frontmatter at the top of markdown."""
    frontmatter = {}
    body = content

    pattern = r"^---\s*\n(.*?)\n---\s*\n"
    match = re.match(pattern, content, re.DOTALL)
    if match:
        fm_text = match.group(1)
        body = content[match.end():]
        for line in fm_text.splitlines():
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if ":" in line:
                key, val = line.split(":", 1)
                key = key.strip()
                val = val.strip().strip('"').strip("'")
                frontmatter[key] = val

    return frontmatter, body


def transform_latex_math(m: str) -> str:
    """Transform LaTeX math commands into clean Unicode/HTML representation."""
    # 1. Strip \text{...}, \mathrm{...}, etc.
    m = re.sub(r"\\(?:text|mathrm|mathbf|mathit)\s*\{([^}]*)\}", r"\1", m)
    # 2. Degree symbols
    m = re.sub(r"\^\{\s*\\circ\s*\}|\^\\circ", "°", m)
    m = re.sub(r"\\circ", "°", m)
    # 3. Greek & special symbols
    m = re.sub(r"\\Phi\s*", "Φ", m)
    m = re.sub(r"\\phi\s*", "φ", m)
    m = re.sub(r"\\Omega|\\ohm", "Ω", m)
    m = re.sub(r"\\mu\s*", "μ", m)
    m = re.sub(r"\\Delta", "Δ", m)
    # 4. Math operators
    m = re.sub(r"\\times", "×", m)
    m = re.sub(r"\\pm", "±", m)
    m = re.sub(r"\\mp", "∓", m)
    m = re.sub(r"\\approx", "≈", m)
    m = re.sub(r"\\sim", "~", m)
    m = re.sub(r"\\leq|\\le", "≤", m)
    m = re.sub(r"\\geq|\\ge", "≥", m)
    m = re.sub(r"\\neq", "≠", m)
    m = re.sub(r"\\cdot", "·", m)
    m = re.sub(r"\\rightarrow|\\to", "→", m)
    m = re.sub(r"\\leftarrow", "←", m)
    # 5. Superscripts and subscripts
    m = re.sub(r"\^\{([^}]+)\}", r"<sup>\1</sup>", m)
    m = re.sub(r"_\{([^}]+)\}", r"<sub>\1</sub>", m)
    # 6. Spacing macros
    m = re.sub(r"\\(?:,|;|quad|qquad|\s)", " ", m)
    # 7. Collapse spaces
    m = re.sub(r"\s+", " ", m).strip()
    return m


def sanitize_latex_in_text(text: str) -> str:
    """
    Sanitize LaTeX math markup in text while protecting code blocks and inline code.
    Replaces $$...$$ and $...$ with clean plain-text/Unicode/HTML tags.
    Preserves currency references like $4 to $5, $100.
    """
    # Protect fenced code blocks (```...```)
    code_blocks = []
    def save_code_block(match):
        code_blocks.append(match.group(0))
        return f"__HACKADAY_CODEBLOCK_{len(code_blocks) - 1}__"

    text = re.sub(r"```.*?```", save_code_block, text, flags=re.DOTALL)

    # Protect inline code (`...`)
    inline_codes = []
    def save_inline_code(match):
        inline_codes.append(match.group(0))
        return f"__HACKADAY_INLINE_{len(inline_codes) - 1}__"

    text = re.sub(r"`[^`\n]+`", save_inline_code, text)

    # 1. Display math $$...$$
    text = re.sub(r"\$\$(.+?)\$\$", lambda m: transform_latex_math(m.group(1)), text, flags=re.DOTALL)

    # 2. Inline math $...$
    def replace_inline_math(match):
        inner = match.group(1)
        # Check if it contains LaTeX syntax, Greek letters, or math operators
        has_latex = ("\\" in inner or any(k in inner for k in ["±", "×", "≤", "≥", "≠", "≈", "^", "_"]))
        if has_latex:
            return transform_latex_math(inner)
        # If it doesn't look like LaTeX math, keep as-is (e.g., currency)
        return match.group(0)

    text = re.sub(r"(?<!\\)\$(?!\s)([^$\n]+?)(?<!\s)\$", replace_inline_math, text)

    # Restore inline code
    for i, code in enumerate(inline_codes):
        text = text.replace(f"__HACKADAY_INLINE_{i}__", code)

    # Restore code blocks
    for i, block in enumerate(code_blocks):
        text = text.replace(f"__HACKADAY_CODEBLOCK_{i}__", block)

    return text


def convert_markdown_to_html(md_text: str) -> str:
    """Convert markdown body to raw HTML."""
    if HAS_MARKDOWN:
        return markdown.markdown(
            md_text,
            extensions=["fenced_code", "tables", "nl2br"]
        )
    else:
        # Minimal fallback parser if markdown package is missing
        lines = md_text.splitlines()
        html_lines = []
        for line in lines:
            if line.startswith("## "):
                html_lines.append(f"<h2>{line[3:].strip()}</h2>")
            elif line.startswith("# "):
                html_lines.append(f"<h1>{line[2:].strip()}</h1>")
            elif line.startswith("- "):
                html_lines.append(f"<li>{line[2:].strip()}</li>")
            elif line.startswith("> "):
                html_lines.append(f"<blockquote>{line[2:].strip()}</blockquote>")
            elif line.strip():
                html_lines.append(f"<p>{line.strip()}</p>")
        return "\n".join(html_lines)


def style_for_hackaday(html_content: str, frontmatter: dict, image_map: dict, cdn_base_url: str = CDN_PROVIDERS[DEFAULT_CDN_PROVIDER]) -> str:
    """
    Post-process HTML into a polished, Hackaday-ready snippet with:
    - Inline styling for headers, blockquotes, code, and images
    - Absolute URL replacements for CDN / image hosting
    - Hero image block and call-to-action footer
    """
    # 1. Style Headers (h2)
    h2_style = 'style="border-bottom: 2px solid #00f0ff; padding-bottom: 4px; margin-top: 28px; margin-bottom: 14px; font-weight: 600; color: #111;"'
    html_content = re.sub(r"<h2>(.*?)</h2>", rf"<h2 {h2_style}>\1</h2>", html_content)

    # 2. Style Blockquotes
    bq_style = 'style="border-left: 4px solid #10b981; padding: 10px 16px; margin: 18px 0; background: rgba(16, 185, 129, 0.08); border-radius: 0 4px 4px 0;"'
    html_content = re.sub(r"<blockquote>\s*", f"<blockquote {bq_style}>", html_content)

    # 3. Style Pre / Code blocks (handles classes like <code class="language-c">)
    pre_style = 'style="background: #1e1e1e; color: #d4d4d4; padding: 14px 18px; border-radius: 6px; overflow-x: auto; font-family: Consolas, monospace; font-size: 0.9em; margin: 18px 0; line-height: 1.5;"'
    html_content = re.sub(r"<pre><code([^>]*)>", rf'<pre {pre_style}><code\1>', html_content)

    # 4. Style Lists
    html_content = re.sub(r"<ul>", '<ul style="line-height: 1.8; margin-left: 20px; margin-bottom: 18px;">', html_content)

    # 5. Handle Markdown Images: exactly once
    def replace_image(match):
        alt = match.group(1)
        src = match.group(2)
        caption = match.group(3) if match.group(3) else alt

        # Resolve image filename and map to absolute CDN URL
        img_name = Path(src).name
        actual_name = image_map.get(img_name, img_name)
        if not (actual_name.endswith(".jpg") or actual_name.endswith(".png")):
            actual_name = Path(actual_name).stem + ".jpg"

        abs_url = f"{cdn_base_url}/{actual_name}"
        caption_html = f'<p style="font-size: 0.85em; color: #666; margin-top: 6px; margin-bottom: 0;"><em>{caption}</em></p>' if caption else ""

        return (
            f'<!-- Image: {alt} -->\n'
            f'<div style="text-align: center; margin: 22px 0;">'
            f'<img src="{abs_url}" alt="{alt}" style="max-width: 100%; height: auto; border-radius: 4px; box-shadow: 0 2px 8px rgba(0,0,0,0.12);">'
            f'{caption_html}'
            f'</div>'
        )

    # Match <p><img alt="alt" src="src" /></p>
    img_pattern = r'<p>\s*<img\s+alt=["\'](.*?)["\']\s+src=["\'](.*?)["\'](?:\s+title=["\'](.*?)["\'])?\s*/?>\s*</p>'
    html_content = re.sub(img_pattern, replace_image, html_content)

    # 6. Build Top Section (Hero image & summary)
    top_section = ""
    hero_image = frontmatter.get("hero_image")
    if hero_image:
        hero_filename = Path(hero_image).name
        actual_hero = image_map.get(hero_filename, hero_filename)
        if not (actual_hero.endswith(".jpg") or actual_hero.endswith(".png")):
            actual_hero = Path(actual_hero).stem + ".jpg"
        hero_url = f"{cdn_base_url}/{actual_hero}"

        top_section += (
            f'<div style="text-align: center; margin-bottom: 24px;">\n'
            f'  <img src="{hero_url}" alt="Hero Image" style="max-width: 100%; height: auto; border-radius: 6px; box-shadow: 0 4px 16px rgba(0,0,0,0.15);">\n'
            f'</div>\n\n'
        )

    summary = frontmatter.get("summary")
    if summary:
        top_section += f'<p style="font-size: 1.1em; line-height: 1.6; color: #222;"><strong>{summary}</strong></p>\n\n'

    # 7. Fixed Call-to-Action (CTA) Footer (Clean GitHub-only link)
    cta_footer = """
<hr style="margin: 36px 0 20px; border: 0; border-top: 1px solid #e2e8f0;">
<p style="text-align: center; font-size: 0.95em; color: #475569; margin: 24px 0 12px;">
  📂 <strong>Open Source Hardware &amp; Code:</strong> <a href="https://github.com/Masafuro/ADX" target="_blank" style="color: #0284c7; text-decoration: underline;">GitHub Repository (Masafuro/ADX)</a>
</p>
"""

    return f"<!-- Hackaday Project Log Snippet (Generated by build_hackaday.py) -->\n{top_section}{html_content}\n{cta_footer}"


def build_single_post(post_path: Path, dirs: dict, image_map: dict, cdn_provider: str = DEFAULT_CDN_PROVIDER, stdout_mode: bool = False):
    """Compile one markdown post into a Hackaday HTML snippet."""
    print(f"\n[Post] Building: {post_path.name}")
    content = post_path.read_text(encoding="utf-8")
    frontmatter, body = parse_frontmatter(content)

    # Sanitize any LaTeX math markup in frontmatter summary and body
    if "summary" in frontmatter:
        frontmatter["summary"] = sanitize_latex_in_text(frontmatter["summary"])
    body = sanitize_latex_in_text(body)

    cdn_base_url = CDN_PROVIDERS.get(cdn_provider, CDN_PROVIDERS[DEFAULT_CDN_PROVIDER])
    html_body = convert_markdown_to_html(body)
    styled_html = style_for_hackaday(html_body, frontmatter, image_map, cdn_base_url=cdn_base_url)

    # Save to dist/
    dirs["dist"].mkdir(parents=True, exist_ok=True)
    out_file = dirs["dist"] / f"{post_path.stem}.html"
    out_file.write_text(styled_html, encoding="utf-8")

    print(f"  -> Generated Hackaday Snippet: docs/blog/dist/{out_file.name} ({len(styled_html)} bytes)")

    if stdout_mode:
        print("\n" + "=" * 50 + " HTML SNIPPET " + "=" * 50)
        print(styled_html)
        print("=" * 114)

    return out_file


def main():
    parser = argparse.ArgumentParser(description="Compile ADX Weekly Markdown logs into Hackaday.io HTML snippets.")
    parser.add_argument("--date", help="Specify post date to compile (e.g. 2026-10-22)")
    parser.add_argument("--cdn", choices=list(CDN_PROVIDERS.keys()), default=DEFAULT_CDN_PROVIDER,
                        help="Image CDN provider (default: jsdelivr). Options: jsdelivr, raw, pages")
    parser.add_argument("--stdout", action="store_true", help="Print generated HTML to stdout")
    parser.add_argument("--skip-images", action="store_true", help="Skip image optimization")
    args = parser.parse_args()

    dirs = get_base_dirs()
    print("=== ADX Hackaday.io Weekly DevLog Compiler ===")
    print(f"Blog Root: {dirs['blog']}")
    print(f"Image CDN: {args.cdn} ({CDN_PROVIDERS[args.cdn]})")

    # Step 1: Process images
    image_map = {}
    if not args.skip_images:
        print("\n--- Step 1: Optimizing Raw Images ---")
        image_map = process_all_raw_images(dirs)
    else:
        print("\n--- Step 1: Image Optimization Skipped ---")

    # Step 2: Compile Markdown Post(s)
    print("\n--- Step 2: Compiling Markdown Posts ---")
    posts_dir = dirs["posts"]

    if args.date:
        target_post = posts_dir / f"{args.date}.md"
        if not target_post.exists():
            print(f"[Error] Post not found: {target_post}", file=sys.stderr)
            sys.exit(1)
        build_single_post(target_post, dirs, image_map, cdn_provider=args.cdn, stdout_mode=args.stdout)
    else:
        posts = sorted(list(posts_dir.glob("*.md")), reverse=True)
        if not posts:
            print("  No posts found in docs/blog/posts/. Place a YYYY-MM-DD.md file to compile.")
            return

        for post in posts:
            build_single_post(post, dirs, image_map, cdn_provider=args.cdn, stdout_mode=args.stdout)

    print("\n[Done] All tasks completed successfully.")


if __name__ == "__main__":
    main()
