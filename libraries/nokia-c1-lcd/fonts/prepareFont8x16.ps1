
param([string]$InputFile="image.png")
$OutputFile="font8x16.hpp"



# Polyfill for bit shifts on Powershell 2.0
Function Enable-BitShift
{
    # Big thanks to Matt Graeber for showing that it is possible to assemble .NET methods with CIL opcodes
    # and for the overall fueling of the fire that is my PowerShell obsession

    #region Create TypeBuilder
    $Domain          = [AppDomain]::CurrentDomain
    $DynAssembly     = New-Object System.Reflection.AssemblyName -ArgumentList @('PoShBitwiseAssembly')
    $AssemblyBuilder = $Domain.DefineDynamicAssembly($DynAssembly, 
                                                     [System.Reflection.Emit.AssemblyBuilderAccess]::Run)
    $ModuleBuilder   = $AssemblyBuilder.DefineDynamicModule('PoShBitwiseModule', $False)
    $TypeBuilder     = $ModuleBuilder.DefineType('PoShBitwiseBuilder', 
                                                 [System.Reflection.TypeAttributes]'Public, Sealed, AnsiClass, AutoClass',
                                                 [Object])
    #endregion

    #region Define Lsh Method
    $MethodBuilder   = $TypeBuilder.DefineMethod('Lsh', 
                                                 [System.Reflection.MethodAttributes]'Public,Static,HideBySig,NewSlot', 
                                                 [Int32], 
                                                 [type[]]([Int32],[UInt32]))
    $MethodBuilder.SetImplementationFlags([System.Reflection.MethodImplAttributes]::IL)
    $ILGen = $MethodBuilder.GetILGenerator(2)
    $ILGen.Emit([Reflection.Emit.OpCodes]::Ldarg_0)
    $ILGen.Emit([Reflection.Emit.OpCodes]::Ldarg_1)
    $ILGen.Emit([Reflection.Emit.OpCodes]::Shl)
    $ILGen.Emit([Reflection.Emit.OpCodes]::Ret)
    #endregion

    #region Define Rsh Method
    $MethodBuilder   = $TypeBuilder.DefineMethod('Rsh', 
                                                 [System.Reflection.MethodAttributes]'Public,Static,HideBySig,NewSlot', 
                                                 [Int32], 
                                                 [type[]]([Int32],[UInt32]))
    $MethodBuilder.SetImplementationFlags([System.Reflection.MethodImplAttributes]::IL)
    $ILGen = $MethodBuilder.GetILGenerator(2)
    $ILGen.Emit([Reflection.Emit.OpCodes]::Ldarg_0)
    $ILGen.Emit([Reflection.Emit.OpCodes]::Ldarg_1)
    $ILGen.Emit([Reflection.Emit.OpCodes]::Shr)
    $ILGen.Emit([Reflection.Emit.OpCodes]::Ret)
    #endregion

    # Generate RuntimeType and assign to global variable
    $Global:Bitwise = $TypeBuilder.CreateType()
}
Enable-BitShift



Write-Host "Loading image"
[void] [System.Reflection.Assembly]::LoadWithPartialName("System.drawing")
$bitMap = [System.Drawing.Bitmap]::FromFile((Get-Item $InputFile).fullname)
$bytes = New-Object byte[] ($bitMap.Height * $bitMap.Width * 2)

Write-Host "Preparing data"
Write-Output @'
#pragma once

const uint8_t font8x16[] PROGMEM = {
'@ | Out-File -Encoding "ASCII" -FilePath $OutputFile
$i = 0
$row = 2
$col = 0
While ($i -lt (6 * 16)) {
    $line = "`t"
    Foreach ($y in ($row * 17)..($row * 17 + 16 - 1)) {
        $value = 0
        Foreach ($x in ($col * 9)..($col * 9 + 8 - 1)) {
            $pixel = $bitMap.GetPixel($x, $y)
            $bitpos = $x % 9
            if ($pixel.R -ge 127) {
                $value = $value -bor $Bitwise::Lsh(1, $bitpos)
            }
        }
        $line += "0x{0:x2}," -f $value
    }
    $line += "// [{0}] '{1}' ({2}) " -f $i, [char]($i + 32), ($i + 32)
    Write-Output $line | Out-File -Encoding "ASCII" -FilePath $OutputFile -Append
    $i += 1

    $col += 1
    if ($col -gt 15) {
        $col = 0
        $row += 1
    }
}
Write-Output @'
};
'@ | Out-File -Encoding "ASCII" -FilePath $OutputFile -Append