# Git Commands
## git操作原理

![img](https://www.runoob.com/wp-content/uploads/2015/02/git-command.jpg)

- workspace：工作区

- staging area：暂存区/缓存区

- local repository：版本库或本地仓库

- remote repository：远程仓库

## 提交与修改

`git add` 添加文件到暂存区 

`git commit` 提交暂存区到本地仓库

`git push` 上传远程代码并合并

`git pull` 下载远程代码并合并

`git fetch` 从远程获取代码库

`git status`查看仓库当前的状态，显示有变更的文件

`git diff` 比较文件的不同，即暂存区和工作区的差异。

`git reset` 回退版本。

`git rm` 删除工作区文件。

`git mv` 移动或重命名工作区文件。

## 创建分支

`git branch branchname` 创建新分支

`git checkout branchname` 切换分支到branchname

`git checkout master` 、`git merge branchname` 将分支更改合并到主分支

`git branch -d branchname` 删除分支

## 提交日志

`git log` 查看历史提交记录

`git blame <file>` 以列表形式查看指定文件的历史修改记录
