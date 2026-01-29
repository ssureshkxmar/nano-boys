@echo off
set IP=3.110.86.38
set KEY=myserverkey.pem

echo ===================================================
echo   HC-LEP AWS DEPLOYMENT KICKOFF
echo   Target: %IP%
echo ===================================================

:: 1. Fix Key Permissions (Critical for Windows SSH)
echo [1/5] Securing key permissions...
icacls %KEY% /reset >nul 2>&1
icacls %KEY% /grant:r "%USERNAME%":"(R)" >nul 2>&1
icacls %KEY% /inheritance:r >nul 2>&1

:: 2. Wait for Server to Boot
echo [2/5] Waiting for server to be ready (5s)...
timeout /t 5 /nobreak >nul

:: 3. Copy Files
echo [3/5] Uploading project files...
scp -o StrictHostKeyChecking=no -i %KEY% server.py ubuntu@%IP%:/home/ubuntu/
scp -o StrictHostKeyChecking=no -i %KEY% -r templates ubuntu@%IP%:/home/ubuntu/
scp -o StrictHostKeyChecking=no -i %KEY% -r static ubuntu@%IP%:/home/ubuntu/

:: 4. Install Dependencies
echo [4/5] Installing Python & Libraries (this may take a minute)...
ssh -o StrictHostKeyChecking=no -i %KEY% ubuntu@%IP% "sudo apt update && sudo apt install -y python3-pip python3-venv && python3 -m venv venv && source venv/bin/activate && pip install flask pycryptodome"

:: 5. Run Server
echo [5/5] Launching Remote Server...
ssh -o StrictHostKeyChecking=no -i %KEY% ubuntu@%IP% "source venv/bin/activate && nohup python server.py > server.log 2>&1 &"

echo ===================================================
echo   DEPLOYMENT SUCCESSFUL!
echo   Dashboard: http://%IP%:5000
echo ===================================================
