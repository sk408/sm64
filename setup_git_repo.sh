#!/bin/bash
# Bash script to initialize Git repository and push to GitHub

echo -e "\e[32mInitializing Git repository and pushing to GitHub...\e[0m"

# Initialize Git repository
git init
if [ $? -ne 0 ]; then
    echo -e "\e[31mError initializing Git repository\e[0m"
    exit 1
fi

# Add all files to the repository
git add .
if [ $? -ne 0 ]; then
    echo -e "\e[31mError adding files to Git\e[0m"
    exit 1
fi

# Commit the files
git commit -m "Initial commit of Pico-ASHA project"
if [ $? -ne 0 ]; then
    echo -e "\e[31mError committing files\e[0m"
    exit 1
fi

# Link to GitHub repository
git remote add origin https://github.com/sk408/sm64.git
if [ $? -ne 0 ]; then
    echo -e "\e[31mError adding remote\e[0m"
    exit 1
fi

# Push to GitHub (try main branch first)
echo -e "\e[34mAttempting to push to main branch...\e[0m"
git push -u origin main
if [ $? -ne 0 ]; then
    # If main fails, try master branch
    echo -e "\e[33mPushing to main branch failed, trying master branch...\e[0m"
    git push -u origin master
    if [ $? -ne 0 ]; then
        echo -e "\e[31mError pushing to GitHub. Make sure the repository exists and you have proper permissions.\e[0m"
        exit 1
    fi
fi

echo -e "\e[32mSuccessfully pushed to GitHub repository!\e[0m"
echo -e "\e[36mYour repository is now available at: https://github.com/sk408/sm64\e[0m" 