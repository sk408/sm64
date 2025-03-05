# PowerShell script to initialize Git repository and push to GitHub
Write-Host "Initializing Git repository and pushing to GitHub..." -ForegroundColor Green

# Initialize Git repository
git init
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error initializing Git repository" -ForegroundColor Red
    exit 1
}

# Add all files to the repository
git add .
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error adding files to Git" -ForegroundColor Red
    exit 1
}

# Commit the files
git commit -m "Initial commit of Pico-ASHA project"
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error committing files" -ForegroundColor Red
    exit 1
}

# Link to GitHub repository
git remote add origin https://github.com/sk408/sm64.git
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error adding remote" -ForegroundColor Red
    exit 1
}

# Push to GitHub (try main branch first)
Write-Host "Attempting to push to main branch..." -ForegroundColor Blue
$pushResult = git push -u origin main
if ($LASTEXITCODE -ne 0) {
    # If main fails, try master branch
    Write-Host "Pushing to main branch failed, trying master branch..." -ForegroundColor Yellow
    git push -u origin master
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Error pushing to GitHub. Make sure the repository exists and you have proper permissions." -ForegroundColor Red
        exit 1
    }
}

Write-Host "Successfully pushed to GitHub repository!" -ForegroundColor Green
Write-Host "Your repository is now available at: https://github.com/sk408/sm64" -ForegroundColor Cyan 