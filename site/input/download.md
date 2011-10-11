menu-position: 4
---

* [windows binary](/sharme/download/sharme.exe)
({%
import os
print(os.path.getsize("input/download/sharme.exe")) / 1024
%} KB)
* [windows binary compressed with upx](/sharme/download/sharme_upx.exe)
({%
import os
print(os.path.getsize("input/download/sharme_upx.exe")) / 1024
%} KB)
* [source tarball](/sharme/download/sharme-0.1.tar.gz)
({%
import os
print(os.path.getsize("input/download/sharme_upx.exe")) / 1024
%} KB)
* [source repo](https://github.com/rpedroso/sharme)

***

> See [build](/sharme/documentation/build.html) to learn how to build from source
and [user documentation](/sharme/documentation/user.html) how to use it.
