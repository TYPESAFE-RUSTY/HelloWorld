$src = "./src/shaders"
$out = "../../spirv/"

$files = Get-ChildItem($src) -File

for ($i = 0; $i -lt $files.Count; $i++) {
    $outf = $files[$i].ToString() + ".spv"
    & glslc $files[$i] -o $outf
    if (!(Test-Path -Path $out)) {
        New-Item -Path $out -ItemType Directory -Force
    }
    & Move-Item $outf $out -Force
}