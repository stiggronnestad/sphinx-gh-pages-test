# Array of project directories
$Projects = @("C:\dev\evert-firmware-pmu\inverter", "C:\dev\evert-firmware-pmu\boost-converter")

# Loop through each project
foreach ($Project in $Projects) {
    Write-Host "Building project in $Project..." -ForegroundColor Cyan

    # Change to the project directory
    Push-Location $Project

    # Run the build command
    $BuildCommand = "cmake --build build\Debug --parallel 8 --clean-first"
    Invoke-Expression $BuildCommand

    # Check if the build succeeded
    if ($LASTEXITCODE -ne 0) {
        Push-Location "C:\dev\evert-firmware-pmu\utils"
        Write-Host "Build failed for $Project. Exiting..." -ForegroundColor Red
        Exit 1
    }

    # Return to the root workspace
    Write-Host "Build completed for $Project." -ForegroundColor Green
}

Push-Location "C:\dev\evert-firmware-pmu\utils"
Write-Host "All projects built successfully!" -ForegroundColor DarkBlue
