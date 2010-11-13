Attribute VB_Name = "moxa7gate"
'СОЗДАНИЕ КОНФИГУРАЦИОННОГО СКРИПТА ДЛЯ ЗАПУСКА ШЛЮЗА MODBUS moxa7gate В КОММУНИКАЦИОННОМ КОНТРОЛЛЕРЕ MOXA UC-7410

Public Const SETTObjectRow As Integer = 4
Public Const SETTLocationRow As Integer = 5
Public Const SETTDesignationRow As Integer = 6
Public Const SETTNetworkNameRow As Integer = 7
Public Const SETTIPAddressRow As Integer = 8
Public Const SETTConfVersionRow As Integer = 9
Public Const SETTConfUCModelRow As Integer = 10
Public Const SETTTCPPortRow As Integer = 14
Public Const SETTMBAddressRow As Integer = 15
Public Const SETTStatusInfoBlockRow As Integer = 16
Public Const SETTShowDataFlowRow As Integer = 21
Public Const SETTShowSysMessagesRow As Integer = 22
Public Const SETTWatchdogTimerRow As Integer = 23
Public Const SETTBuzzerRow As Integer = 24

Public Const SLGWPortColumn As Integer = 1
Public Const SLGWModeColumn As Integer = 2
Public Const SLModeColumn As Integer = 3
Public Const SLSpeedColumn As Integer = 4
Public Const SLParityColumn As Integer = 5
Public Const SLTimeoutColumn As Integer = 6
Public Const SLTCPPortColumn As Integer = 7
Public Const SLCommentColumn As Integer = 8

Public Const PTAddressColumn As Integer = 2
Public Const PTMBTableColumn As Integer = 3
Public Const PTStartColumn As Integer = 4
Public Const PTLengthColumn As Integer = 5
Public Const PTOffsetColumn As Integer = 6
Public Const PTPortColumn As Integer = 7
Public Const PTDelayColumn As Integer = 8
Public Const PTCriticalColumn As Integer = 9
Public Const PTCommentColumn As Integer = 10

Public Const RTMAddressColumn As Integer = 2
Public Const RTMMBTableColumn As Integer = 3
Public Const RTMStartColumn As Integer = 4
Public Const RTMLengthColumn As Integer = 5
Public Const RTMAddrShiftColumn As Integer = 6
Public Const RTMPortColumn As Integer = 7
Public Const RTMCommentColumn As Integer = 8

Public Const TSMBAddressColumn As Integer = 2
Public Const TSIPAddressColumn As Integer = 3
Public Const TSTCPPortColumn As Integer = 4
Public Const TSAddrShiftColumn As Integer = 5
Public Const TSPortColumn As Integer = 6
Public Const TSCommentColumn As Integer = 7

Public Const EXCPTOnColumn As Integer = 2
Public Const EXCPTPrmColumn As Integer = 3

Public ErrorCount As Integer

Public Sub WriteSH()

Dim str As String
Dim res As Boolean
Dim first As Boolean

'**** ВЫПОЛНЯЕМ ПРОВЕРКУ КОНФИГУРАЦИИ

Call CheckSH
If ErrorCount <> 0 Then
  If Not MsgBox("Найдено" + CStr(ErrorCount) + "ошибок. Все равно продолжить?", vbYesNo, Внимание) Then
'    Exit Sub
    End If
  End If

'**** ОТКРЫВАЕМ ФАЙЛ ДЛЯ ЗАПИСИ

FileToOpen = Application.GetSaveAsFilename(InitialFileName:=FILE_NAME, fileFilter:="Linux Shell Scripts (*.sh), *.sh")

If FileToOpen <> False Then
    Open FileToOpen For Output Access Write Lock Write As #1
Else
    MsgBox "Ошибка при открытии файла. Повторите попытку позже."
    Exit Sub
End If

'**** ЗАПИСЬ ОБЩИХ ПАРАМЕТРОВ НАСТРОЙКИ ШЛЮЗА ****

Set ws = Worksheets("Шлюз")
ws.Activate

' инкрементируем номер версии
ws.Cells(SETTConfVersionRow, 2).Value = CStr("1.") + CStr(CDec(Mid(ws.Cells(SETTConfVersionRow, 2).Value, 3)) + 1)

Print #1, "#! /bin/sh"
Print #1, "#"
Print #1, "# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2"
Print #1, "# SEM-ENGINEERING, BRYANSK " + CStr(Year(Date))
Print #1, "#"
Print #1, "# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY"
Print #1, "# AT " + CStr(Time) + " " + CStr(Date) '!!! добавить аналогичный код в переменную для передачи в программу
Print #1, "#"
Print #1, "# Object: " + ws.Cells(SETTObjectRow, 2).Value
Print #1, "# Location: " + ws.Cells(SETTLocationRow, 2).Value + " " + ws.Cells(SETTDesignationRow, 2).Value
Print #1, "#"
Print #1, "# File Version: " + CStr(ws.Cells(SETTConfVersionRow, 2).Value)
Print #1, "#"
Print #1, "# Network Name: " + ws.Cells(SETTNetworkNameRow, 2).Value
Print #1, "# Network Address: " + ws.Cells(SETTIPAddressRow, 2).Value
Print #1, "#"
Print #1, " "

If ws.Cells(SETTConfUCModelRow, 2).Value = "MOXA UC-7410" Then
    Print #1, "/root/moxa7gate \"
Else
    Print #1, "/home/root/moxa7gate_plus \"
End If

If ws.Cells(SETTTCPPortRow, 2).Value <> "" And ws.Cells(SETTTCPPortRow, 2).Value <> 0 Then
    Print #1, "--tcp_port " + CStr(ws.Cells(SETTTCPPortRow, 2).Value) + " \"
End If

If ws.Cells(SETTMBAddressRow, 2).Value <> "" And ws.Cells(SETTMBAddressRow, 2).Value <> 0 Then
    Print #1, "--modbus_address " + CStr(ws.Cells(SETTMBAddressRow, 2).Value) + " \"
End If

If ws.Cells(SETTStatusInfoBlockRow, 2).Value <> "" And ws.Cells(SETTStatusInfoBlockRow, 2).Value <> 0 Then
    Print #1, "--status_info " + CStr(ws.Cells(SETTStatusInfoBlockRow, 2).Value) + " \"
End If

If ws.Cells(SETTShowDataFlowRow, 2).Value = "Да" Then
    Print #1, "--show_data_flow \"
End If

If ws.Cells(SETTShowSysMessagesRow, 2).Value = "Да" Then
    Print #1, "--show_sys_messages \"
End If

If ws.Cells(SETTWatchdogTimerRow, 2).Value = "Да" Then
    Print #1, "--watchdog_timer \"
End If

If ws.Cells(SETTBuzzerRow, 2).Value = "Да" Then
    Print #1, "--use_buzzer \"
End If

str = "--Object " + """"
str = str + CStr(ws.Cells(SETTObjectRow, 2).Value) + """ \"
Print #1, str

str = "--Location " + """"
str = str + CStr(ws.Cells(SETTLocationRow, 2).Value + " " + ws.Cells(SETTDesignationRow, 2).Value) + """ \"
Print #1, str

str = "--confVersion " + """"
str = str + CStr(ws.Cells(SETTConfVersionRow, 2).Value) + """ \"
Print #1, str

str = "--NetworkName " + """"
str = str + CStr(ws.Cells(SETTNetworkNameRow, 2).Value) + """ \"
Print #1, str

str = "--NetworkAddress " + """"
str = str + CStr(ws.Cells(SETTIPAddressRow, 2).Value) + """ \"
Print #1, str

'**** ЗАПИСЬ ИСКЛЮЧЕНИЙ ****

Set ws = Worksheets("Исключения")
ws.Activate

For i = 2 To 17

    If ws.Cells(i, EXCPTOnColumn).Value = "Да" Then
        Print #1, "--exception " + Format(i - 1) + " " + CStr(ws.Cells(i, EXCPTPrmColumn).Value) + " \"
    End If

Next i

'**** ЗАПИСЬ ПАРАМЕТРОВ КОНФИГУРАЦИИ ПОСЛЕДОВАТЕЛЬНЫХ ИНТЕРФЕЙСОВ ****

Set ws = Worksheets("Serial интерфейсы")
ws.Activate

For i = 2 To 9

res = (ws.Cells(i, SLGWModeColumn).Value <> "") And (ws.Cells(i, SLGWModeColumn).Value <> "ОТКЛЮЧЕН")

    If res Then

        str = "PORT" + Format(i - 1) + " "
        str = str + CStr(ws.Cells(i, SLModeColumn).Value) + " "
        str = str + CStr(ws.Cells(i, SLSpeedColumn).Value) + " "
        str = str + CStr(ws.Cells(i, SLParityColumn).Value) + " "
        str = str + CStr(ws.Cells(i, SLTimeoutColumn).Value) + " "
        str = str + CStr(ws.Cells(i, SLGWModeColumn).Value) + " "
        
        If ws.Cells(i, SLGWModeColumn).Value = "GATEWAY_SIMPLE" Then
            str = str + CStr(ws.Cells(i, SLTCPPortColumn).Value) + " "
        End If
        
        If ws.Cells(i, SLCommentColumn).Value <> "" Then
            str = str + "--desc " + """"
            str = str + CStr(ws.Cells(i, SLCommentColumn).Value) + """ "
        End If
        
        Print #1, str + "\"
    End If

Next i

'**** ЗАПИСЬ ТАБЛИЦЫ ОПРОСА PROXY_TABLE ****

Set ws = Worksheets("PROXY_TABLE")
ws.Activate

first = True

For i = 2 To 129
    If ws.Cells(i, PTAddressColumn).Value = "" Then
        j = i - 2
        Exit For
    End If
Next i

For i = 2 To 129
    
    res = ws.Cells(i, PTAddressColumn).Value <> ""

    If res Then

        If first Then
            Print #1, "PROXY_TABLE " + CStr(j) + " \"
            first = False
        End If
        
        str = CStr(ws.Cells(i, PTStartColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTLengthColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTOffsetColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTPortColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTAddressColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTMBTableColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTDelayColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTCriticalColumn).Value) + " "
        
        If ws.Cells(i, PTCommentColumn).Value <> "" Then
            str = str + "--desc " + """"
            str = str + CStr(ws.Cells(i, PTCommentColumn).Value) + """ "
        End If
        
        Print #1, str + "\"
    End If

Next i

'**** ЗАПИСЬ ТАБЛИЦЫ ВИРТУАЛЬНЫХ УСТРОЙСТВ RTM_TABLE ****

Set ws = Worksheets("RTM_TABLE")
ws.Activate

first = True

For i = 2 To 129
    If ws.Cells(i, RTMStartColumn).Value = "" Then
        j = i - 2
        Exit For
    End If
Next i

For i = 2 To 129
    
    res = ws.Cells(i, RTMStartColumn).Value <> ""

    If res Then

        If first Then
            Print #1, "RTM_TABLE " + CStr(j) + " \"
            first = False
        End If
        
        str = CStr(ws.Cells(i, RTMStartColumn).Value) + " "
        str = str + CStr(ws.Cells(i, RTMLengthColumn).Value) + " "
        str = str + CStr(ws.Cells(i, RTMPortColumn).Value) + " "
        str = str + CStr(ws.Cells(i, RTMAddressColumn).Value) + " "
        str = str + CStr(ws.Cells(i, RTMMBTableColumn).Value) + " "
        
        If ws.Cells(i, RTMAddrShiftColumn).Value <> "" And ws.Cells(i, RTMAddrShiftColumn).Value <> 0 Then
            str = str + "--address_shift " + CStr(ws.Cells(i, RTMAddrShiftColumn).Value) + " "
        End If
        
        If ws.Cells(i, RTMCommentColumn).Value <> "" Then
            str = str + "--desc " + """"
            str = str + CStr(ws.Cells(i, RTMCommentColumn).Value) + """ "
        End If
        
        Print #1, str + "\"
    End If

Next i

'**** ЗАПИСЬ ТАБЛИЦЫ MOBBUS TCP-СЕРВЕРОВ ****

Set ws = Worksheets("Ethernet интерфейсы")
ws.Activate

first = True

For i = 2 To 33
    If ws.Cells(i, TSMBAddressColumn).Value = "" Then
        j = i - 2
        Exit For
    End If
Next i

For i = 2 To 33
    
    res = ws.Cells(i, TSMBAddressColumn).Value <> ""

    If res Then

        If first Then
            Print #1, "TCP_SERVERS " + CStr(j) + " \"
            first = False
        End If
        
        str = CStr(ws.Cells(i, TSIPAddressColumn).Value) + ":"
        str = str + CStr(ws.Cells(i, TSTCPPortColumn).Value) + " "
        str = str + CStr(ws.Cells(i, TSMBAddressColumn).Value) + " "
        str = str + CStr(ws.Cells(i, TSAddrShiftColumn).Value) + " "
        str = str + CStr(ws.Cells(i, TSPortColumn).Value) + " "

        If ws.Cells(i, TSCommentColumn).Value <> "" Then
            str = str + "--desc " + """"
            str = str + CStr(ws.Cells(i, TSCommentColumn).Value) + """ "
        End If
        
        Print #1, str + "\"
    End If

Next i

'**** ВЫПОЛНЕНО, ЗАКРЫВАЕМ ФАЙЛ

Print #1, "&"
Close #1

Set ws = Worksheets("Шлюз")
ws.Activate

End Sub

' НАБОР ФУНКЦИЙ ДЛЯ ПРОВЕРКИ КОРРЕКТНОСТИ ТЕКУЩЕЙ КОНФИГУРАЦИИ ШЛЮЗА

Public Sub CheckSH()
  
  ErrorCount = 0
  Dim tmp As Integer
  
  tmp = ErrorCount
  Call CheckInterfaces
  If ErrorCount <> 0 Then
    MsgBox ("Найдены ошибки в количестве: " + CStr(ErrorCount - tmp))
    End If
  
  tmp = ErrorCount
  Call CheckProxyTable
  If ErrorCount <> 0 Then
    MsgBox ("Найдены ошибки в количестве: " + CStr(ErrorCount - tmp))
    End If
  
  End Sub

Private Sub CheckInterfaces()

Dim counter As Integer

Set ws = Worksheets("Serial интерфейсы")
ws.Activate

Range("B2:H9").Select
Range("B2").Activate
Selection.ClearComments
Selection.Interior.ColorIndex = xlNone
Range("A11:A11").Select
Range("A11").Activate

For i = 2 To 9

Select Case ws.Cells(i, SLGWModeColumn).Value

  Case "GATEWAY_SIMPLE"
    If ws.Cells(i, SLTCPPortColumn).Value = "" Then
      ws.Cells(i, SLTCPPortColumn).Interior.Color = RGB(255, 0, 0)
      ws.Cells(i, SLTCPPortColumn).AddComment ("Не определен номер TCP-порта")
      ErrorCount = ErrorCount + 1
      End If
  
  Case "GATEWAY_ATM"
  
  Case "GATEWAY_RTM"
  
  Case "GATEWAY_PROXY"
    Set ws2 = Worksheets("PROXY_TABLE")
    counter = 0
    For j = 2 To 129
      If ws2.Cells(j, PTPortColumn).Value = ws.Cells(i, SLGWPortColumn).Value Then
        counter = counter + 1
        End If
      Next j
    If counter = 0 Then
      ws.Cells(i, SLGWModeColumn).Interior.Color = RGB(255, 0, 0)
      ws.Cells(i, SLGWModeColumn).AddComment ("В таблице PROXY_TABLE нет записей")
      ErrorCount = ErrorCount + 1
      Else
      ws.Cells(i, SLGWModeColumn).AddComment ("Найдено ") + CStr(counter) + " записи(ей) в таблице PROXY_TABLE"
      End If
  
  Case "BRIDGE_PROXY"
  
  Case "BRIDGE_SIMPLE"
  
  Case Else
  
  End Select
  

Next i



End Sub

Private Sub CheckProxyTable()

Set ws = Worksheets("Serial интерфейсы")
Set ws2 = Worksheets("PROXY_TABLE")
ws2.Activate

Range("B2:J129").Select
Range("B2").Activate
Selection.ClearComments
Selection.Interior.ColorIndex = xlNone
Range("A1:A1").Select
Range("A1").Activate

For j = 2 To 129
  
  If Mid(ws2.Cells(j, PTPortColumn).Value, 1, 1) <> "T" And ws2.Cells(j, PTPortColumn).Value <> "" Then
    k = CInt(Mid(ws2.Cells(j, PTPortColumn).Value, 2, 1)) + 1

    If ws.Cells(k, SLGWModeColumn).Value <> "GATEWAY_PROXY" Then
      ws2.Cells(j, PTPortColumn).Interior.Color = RGB(255, 0, 0)
      ws2.Cells(j, PTPortColumn).AddComment ("Указанный порт находится не в режиме GATEWAY_PROXY")
      ErrorCount = ErrorCount + 1
      End If
    
    End If

  Next j

End Sub


