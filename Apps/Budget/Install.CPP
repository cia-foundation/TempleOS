Cd(__DIR__);;
if (!FileFind("::/Home/Budget",,FUF_JUST_DIRS)) {
  MkDir("::/Home/Budget");
  Copy("Accts.TXT.Z","::/Home/Budget");
}
