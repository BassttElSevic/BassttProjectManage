# PowerShell Git 上传命令脚本
# 用于将项目更新上传到 GitHub 仓库

# 设置工作目录
Set-Location "D:\LIANXI\untitled"

# 1. 检查 git 状态
Write-Host "检查 Git 状态..." -ForegroundColor Green
git status

# 2. 添加所有修改的文件到暂存区
Write-Host "`n添加文件到暂存区..." -ForegroundColor Green
git add README.md

# 3. 提交代码更改
Write-Host "`n提交代码更改..." -ForegroundColor Green
git commit -m "✨ 优化日历动画效果：添加多层次发光、呼吸灯效果和闪光动画

- 改进日历发光边框：使用分层发光效果，更精致柔和
- 添加呼吸灯效果：轻微脉动，增加动态感
- 新增闪光效果：提升视觉层次感
- 优化粒子动画：快速衰减，显示时间更短
- 改进缓动函数：使用平滑的缓出二次缓动
- 更新README.md：添加日历动画优化说明和更新日志"

# 4. 拉取远程分支最新代码（处理分支分歧）
Write-Host "`n拉取远程分支..." -ForegroundColor Green
git pull origin main

# 5. 推送代码到 GitHub
Write-Host "`n推送到 GitHub..." -ForegroundColor Green
git push origin main

Write-Host "`n✅ 上传完成！代码已成功提交到 GitHub。" -ForegroundColor Cyan
Write-Host "仓库地址: https://github.com/BassttElSevic/BassttProjectManage" -ForegroundColor Cyan

