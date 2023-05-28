set out="..\src\bin.h"

echo #pragma warning(disable:4838) > %out%
echo #pragma warning(disable:4309) >> %out%

type "Shadow Protector.exe" | bin2h.exe -c shadow_protector_exe >> %out%
type "Shadow Protector.ini" | bin2h.exe -c shadow_protector_ini >> %out%
