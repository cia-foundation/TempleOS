Cd(__DIR__);;
if (!FileFind("~/Budget",,FUF_JUST_DIRS)) {
  DirMk("~/Budget");
  Copy("Accts.DD.Z","~/Budget");
}
