# 创建repository时如果勾选README.md，就不要执行以下git add README.md命令，若执行了
# 在push时由于远程的未pull，本地的未push，在push时需要force强推，git push -u origin main
# 替换为：git push -u origin main -f
# 在github上新创建一个repository时提示：create a new repository on the command line
echo "# Sublime-Merge-tutorial" >> README.md
git init
git add README.md
git commit -m "first commit"
git branch -M main	
git remote add origin git@github.com:bobokong/Sublime-Merge-tutorial.git
git push -u origin main

# 若出现以下错误
> git push -u origin mian
Permission denied (publickey).
fatal: Could not read from remote repository.

Please make sure you have the correct access rights
and the repository exists.



# https://github.com/kongdd/ubuntu-config/blob/master/1_Git.md

# /usr/bin/ssh-keygen -A
# sudo service ssh --full-restart
ssh-keygen -C kongdd
ssh-copy-id -i ~/.ssh/id_rsa.pub kong@remoteIP
ssh -T git@github.com

# windows format
git config --global core.safecrlf false
git config --global core.autocrlf true

# line ending maters
git config --global core.safecrlf true
git config --global core.autocrlf false
## transform line ending for windows
# git config --global core.autocrlf true

git config --global user.name "Dongdong Kong"
git config --global user.email "kongdd@users.noreply.github.com"
## 记录用户名
git config --global credential.helper store
git config --list --global
# git config --global --unset credential.helper
