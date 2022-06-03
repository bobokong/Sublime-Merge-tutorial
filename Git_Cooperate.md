# 使用Github进行协作

**1. 发出邀请**

   >github.com/username -> username/repository -> Setting -> Collaporators ->  Add people

**2. 被邀请人接受邀请**

**3. 成员git ssh，之后就可以进行同步办公**

   `git clone git@github.com:bobot39/test.git`

**4. 创建分支并切换到新建分支**

   `git checkout -b collaporation`

**5. 将分支同步到远程**

   `git push --set-upstream origin collaporation`

**6. 在分支内修改并将分支与主分支合并**
```vi led.c  
git add led.c  
git commit -m "add led.c"  
git push  
git checkout main
git merge collaporation```
