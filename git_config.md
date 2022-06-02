# Git configuration

创建repository时如果勾选README.md，就不要执行以下git add README.md命令，若执行了
在push时由于远程的未pull，本地的未push，在push时需要force强推，git push -u origin main
替换为：`git push -u origin main -f`
在github上新创建一个repository时提示：create a new repository on the command line

## 本地配置步骤：

1. `git config --global user.name "git username"`

2. `git config --global user.email "user email"`

   查看配置：`git config --list`

3. 生成SSH-KEY

   <!--执行下面命令后会在~/.ssh/目录下生成id_rsa(私钥)和id_rsa.pub(公钥)-->

   `ssh-keygen -t rsa -C "user email" -f ~/.ssh/id_rsa`

4. 添加私钥

   `ssh-add ~/.ssh/id_rsa`

   如果执行ssh-add时提示“Could not open a connection to your authentication agent”，可以先执行命令ssh-agent bash，然后再执行以上代码

## github端配置：

登录github账户 -> settings -> SSH and GPG keys -> new SSH key -> 复制id_rsa.pub里的内容填入

## 测试

`ssh -T git@github.com`
如果成功：`You've successfully authenticated, but GitHub does not provide shell access.`
## git clone

**注意：**git clone时要使用SSH地址，如`git clone git@github.com:bobot39/test1.git`
若是使用的http地址将会无法使用push命令

```bash
echo "# Sublime-Merge-tutorial" >> README.md
git init
git add README.md
git commit -m "first commit"
git branch -M main	
git remote add origin git@github.com:bobokong/Sublime-Merge-tutorial.git
git push -u origin main
```

# 若出现以下错误

> `git push -u origin mian Permission denied (publickey).fatal: Could not read from remote repository.Please make sure you have the correct access rights and the repository exists.`

可参考以下解决方案
<https://stackoverflow.com/questions/12940626/github-error-message-permission-denied-publickey>

# Git初始化配置参考如下
<https://github.com/kongdd/ubuntu-config/blob/master/1_Git.md>

```bash
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
```
