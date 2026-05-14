# 分阶段手动提交说明

当前仓库根目录已经处于 stage2 待提交状态。

推荐顺序：

1. 提交当前根目录中的 stage2 改动：

```powershell
git add -A
git commit -m "stage2: add qt desktop shell and navigation"
```

2. 覆盖 stage3 增量并提交：

```powershell
Copy-Item -Recurse -Force .\stage_increments\stage3_business_crud\* .\
git add -A
git commit -m "stage3: implement business pages and crud dialogs"
```

3. 覆盖 stage4 增量并提交：

```powershell
Copy-Item -Recurse -Force .\stage_increments\stage4_advanced_features\* .\
git add -A
git commit -m "stage4: add resume editor analysis import and ai assistant"
```

4. 覆盖 stage5 增量并提交：

```powershell
Copy-Item -Recurse -Force .\stage_increments\stage5_docs_submission\* .\
git add -A
git commit -m "stage5: add reports documentation and final materials"
```

5. 最后推送：

```powershell
git push origin main
```

注意：`stage_increments` 是给你手动提交用的暂存材料目录，不建议最后提交到远程仓库。
