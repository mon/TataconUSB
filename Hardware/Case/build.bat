SET scad="C:\Program Files\OpenSCAD\openscad.exe"
SET folder=outputs

for %%i in (TataCase_V5,TataCase_V5_Rear,TataCase_V4,TataCase_V4_Rear,TataCase_V5_Thin,TataCase_V5_Thin_Rear) DO (
    %scad% -o "%folder%\%%i.stl" "%%i.scad"
)

pause