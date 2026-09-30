#
# Regenerate assets/icons/directkeycast.ico. Same design as dkui::AppLogo:
# accent-gradient tile, white keycap, "K" legend.
#
[CmdletBinding()]
param(
    [string]$OutFile = (Join-Path (Split-Path -Parent $PSScriptRoot) "assets/icons/directkeycast.ico")
)

$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Drawing

function New-RoundedPath([float]$x, [float]$y, [float]$w, [float]$h, [float]$r) {
    $path = [System.Drawing.Drawing2D.GraphicsPath]::new()
    $d = $r * 2
    $path.AddArc($x, $y, $d, $d, 180, 90)
    $path.AddArc($x + $w - $d, $y, $d, $d, 270, 90)
    $path.AddArc($x + $w - $d, $y + $h - $d, $d, $d, 0, 90)
    $path.AddArc($x, $y + $h - $d, $d, $d, 90, 90)
    $path.CloseFigure()
    return $path
}

function New-IconFrame([int]$s) {
    $bmp = [System.Drawing.Bitmap]::new($s, $s, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
    $g.Clear([System.Drawing.Color]::Transparent)

    $tile = New-RoundedPath 0 0 ($s - 0.5) ($s - 0.5) ($s * 0.24)
    $gradient = [System.Drawing.Drawing2D.LinearGradientBrush]::new(
        [System.Drawing.PointF]::new(0, 0), [System.Drawing.PointF]::new($s, $s),
        [System.Drawing.Color]::FromArgb(0x9C, 0x9A, 0xE6), [System.Drawing.Color]::FromArgb(0x6B, 0x69, 0xD6))
    $g.FillPath($gradient, $tile)

    $capL = $s * 0.20; $capT = $s * 0.20; $capW = $s * 0.60; $capH = $s * 0.54
    $lip = New-RoundedPath $capL ($capT + $s * 0.06) $capW $capH ($s * 0.10)
    $g.FillPath([System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(115, 255, 255, 255)), $lip)
    $face = New-RoundedPath $capL $capT $capW $capH ($s * 0.10)
    $g.FillPath([System.Drawing.Brushes]::White, $face)

    $font = [System.Drawing.Font]::new("Segoe UI", [float]($s * 0.36), [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $format = [System.Drawing.StringFormat]::new()
    $format.Alignment = [System.Drawing.StringAlignment]::Center
    $format.LineAlignment = [System.Drawing.StringAlignment]::Center
    $legend = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(0x5B, 0x57, 0xB8))
    $g.DrawString("K", $font, $legend, [System.Drawing.RectangleF]::new($capL, $capT, $capW, $capH), $format)

    $g.Dispose()
    $stream = [System.IO.MemoryStream]::new()
    $bmp.Save($stream, [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
    return , $stream.ToArray()
}

$sizes = @(16, 20, 24, 32, 40, 48, 64, 256)
$frames = foreach ($size in $sizes) { , (New-IconFrame $size) }

# ICO container with PNG-compressed entries (supported since Windows Vista).
$out = [System.IO.MemoryStream]::new()
$writer = [System.IO.BinaryWriter]::new($out)
$writer.Write([uint16]0)
$writer.Write([uint16]1)
$writer.Write([uint16]$sizes.Count)
$offset = 6 + 16 * $sizes.Count
for ($i = 0; $i -lt $sizes.Count; $i++) {
    $dim = if ($sizes[$i] -ge 256) { 0 } else { $sizes[$i] }
    $writer.Write([byte]$dim)
    $writer.Write([byte]$dim)
    $writer.Write([byte]0)
    $writer.Write([byte]0)
    $writer.Write([uint16]1)
    $writer.Write([uint16]32)
    $writer.Write([uint32]$frames[$i].Length)
    $writer.Write([uint32]$offset)
    $offset += $frames[$i].Length
}
foreach ($frame in $frames) {
    $writer.Write($frame)
}
$writer.Flush()

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $OutFile) | Out-Null
[System.IO.File]::WriteAllBytes($OutFile, $out.ToArray())
Write-Host "Wrote $OutFile"
