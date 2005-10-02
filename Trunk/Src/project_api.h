extern int         prj_get_numconfigs();
extern int         prj_get_numpackages();

extern const char* prj_get_bindir(int pathType, int appendSeparator);
extern const char* prj_get_bindir_for(Package* pkg, int pathType, int appendSeparator);
extern const char* prj_get_cfgname();
extern const char* prj_get_language();
extern const char* prj_get_libdir(int pathType, int appendSeparator);
extern const char* prj_get_libdir_for(Package* pkg, int pathType, int appendSeparator);
extern const char* prj_get_objdir(int pathType, int appendSeparator);
extern const char* prj_get_outdir(int pathType, int appendSeparator);
extern const char* prj_get_outdir_for(Package* pkg, int pathType, int appendSeparator);
extern const char* prj_get_pkgname();
extern const char* prj_get_pkgpath();
extern const char* prj_get_target();
extern const char* prj_get_target_for(Package* pkg);

extern void        prj_select_config(int i);
extern void        prj_select_package(int i);

