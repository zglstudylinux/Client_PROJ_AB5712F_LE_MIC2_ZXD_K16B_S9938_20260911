loadresdir(res);
makeres(res_buf);
makeresdef(res.h);
xcopy(res.h, ../../res.h);
xcopy(effect.c, ../../effect.c);
xcopy(effect.h, ../../effect.h);
save(res_buf, res.bin);
