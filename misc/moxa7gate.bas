Attribute VB_Name = "moxa7gate"
'/***********   MOXA7GATE   *************
'        MODBUS GATEWAY SOFTWARE
'                    Version 1.2
'        SEM-ENGINEERING
'               BRYANSK 2015
'***************************************/

' �������� ����������������� ������� ��� ������� ����� MODBUS moxa7gate
' � ���������������� ����������� MOXA UC-7410(Plus), UC-8410 ������ 1.2

' ���� "����"
' ����� ����������
Public Const SETTObjectRow As Integer = 4
Public Const SETTLocationRow As Integer = 5
Public Const SETTLabelRow As Integer = 6
Public Const SETTNetworkNameRow As Integer = 7
Public Const SETTLAN1AddressRow As Integer = 8
Public Const SETTLAN2AddressRow As Integer = 9
Public Const SETTVersionNumberRow As Integer = 10
Public Const SETTVersionTimeRow As Integer = 11
' ���� MODBUS
Public Const SETTModelRow As Integer = 15
Public Const SETTTCPPortRow As Integer = 16
Public Const SETTStatusInfoBlockRow As Integer = 17
Public Const SETTShowSysMessagesRow As Integer = 18
Public Const SETTShowDataFlowRow As Integer = 19
Public Const SETTMap4Xto2XRow As Integer = 20
Public Const SETTMap3Xto4XRow As Integer = 21
Public Const SETTWatchdogTimerRow As Integer = 22
Public Const SETTBuzzerRow As Integer = 23

' ���� "SERIAL-����������"
Public Const SLGWPortColumn As Integer = 1
Public Const SLGWModeColumn As Integer = 2
Public Const SLModeColumn As Integer = 3
Public Const SLSpeedColumn As Integer = 4
Public Const SLParityColumn As Integer = 5
Public Const SLTimeoutColumn As Integer = 6
Public Const SLTCPPortColumn As Integer = 7
Public Const SLCommentColumn As Integer = 8

' ���� "LANTCP-����������"
Public Const LANTCPIfaceColumn As Integer = 1
Public Const LANTCPIPColumn As Integer = 2
Public Const LANTCPTCPColumn As Integer = 3
Public Const LANTCPUnitIDColumn As Integer = 4
Public Const LANTCPOffsetColumn As Integer = 5
Public Const LANTCPTimeoutColumn As Integer = 6
Public Const LANTCPIP2Column As Integer = 7
Public Const LANTCPTCP2Column As Integer = 8
Public Const LANTCPCommentColumn As Integer = 9

' ���� "������� ������"
Public Const PTIfaceColumn As Integer = 2
Public Const PTMBAddressColumn As Integer = 3
Public Const PTMBTableColumn As Integer = 4
Public Const PTAccessColumn As Integer = 5
Public Const PTStartColumn As Integer = 6
Public Const PTLengthColumn As Integer = 7
Public Const PTOffsetColumn As Integer = 8
Public Const PTDelayColumn As Integer = 9
Public Const PTCriticalColumn As Integer = 10
Public Const PTCommentColumn As Integer = 11

' ���� "���������� ���������"
Public Const RTMIfaceColumn As Integer = 2
Public Const RTMMBAddressColumn As Integer = 3
Public Const RTMMBTableColumn As Integer = 4
Public Const RTMOffsetColumn As Integer = 5
Public Const RTMStartColumn As Integer = 6
Public Const RTMLengthColumn As Integer = 7
Public Const RTMCommentColumn As Integer = 8

' ���� "����������"
Public Const EXCPStageColumn As Integer = 2
Public Const EXCPActionColumn As Integer = 3
Public Const EXCPPrm1Column As Integer = 4
Public Const EXCPPrm2Column As Integer = 5
Public Const EXCPPrm3Column As Integer = 6
Public Const EXCPPrm4Column As Integer = 7
Public Const EXCPCommentColumn As Integer = 8

' ���� "����� �������"
Public Const ATMFirstRow As Integer = 3
Public Const ATMRowsAmount As Integer = 32
Public Const ATMFirstColumn As Integer = 2
Public Const ATMColumnsAmount As Integer = 8

Public ErrorCount As Integer

Public Sub WriteSH()

Dim str As String
Dim res As Boolean
Dim first As Boolean

'**** ��������� �������� ������������

'Call CheckSH
'If ErrorCount <> 0 Then
'  If Not MsgBox("�������" + CStr(ErrorCount) + "������. ��� ����� ����������?", vbYesNo, ��������) Then
'    Exit Sub
'    End If
'  End If

'**** ��������� ���� ��� ������

FileToOpen = Application.GetSaveAsFilename(InitialFileName:=FILE_NAME, fileFilter:="Bash Scripts (*.bash), *.bash")

If FileToOpen <> False Then
    Open FileToOpen For Output Access Write Lock Write As #1
Else
    MsgBox "������ ��� �������� �����. ��������� ������� �����."
    Exit Sub
End If

'**** ������ ����� ���������� ��������� ����� ****

Set ws = Worksheets("����")
'ws.Activate

' �������������� ����� ������, �������� ������� �����
ws.Cells(SETTVersionNumberRow, 2).Value = CStr(Mid(ws.Cells(SETTVersionNumberRow, 2).Value, 1, 2)) + CStr(CDec(Mid(ws.Cells(SETTVersionNumberRow, 2).Value, 3)) + 1)

If Mid(CStr(Time), 2, 1) = ":" Then
  ws.Cells(SETTVersionTimeRow, 2).Value = CStr(Date) + " 0" + CStr(Time)
Else
  ws.Cells(SETTVersionTimeRow, 2).Value = CStr(Date) + " " + CStr(Time)
End If


Print #1, "#!/bin/bash"
Print #1, "#"
Print #1, "# MOXA7GATE MODBUS GATEWAY SOFTWARE VERSION 1.2"
Print #1, "# AO NPO SEM, BRYANSK " + CStr(Year(Date))
Print #1, "#"
Print #1, "# OBJECT: " + ws.Cells(SETTObjectRow, 2).Value
Print #1, "# LOCATION: " + ws.Cells(SETTLocationRow, 2).Value
Print #1, "# LABEL: " + ws.Cells(SETTLabelRow, 2).Value
Print #1, "#"
Print #1, "# THIS CONFIGURATION FILE GENERATED AUTOMATICALLY"
Print #1, "# AT " + ws.Cells(SETTVersionTimeRow, 2).Value
Print #1, "# VERSION " + CStr(ws.Cells(SETTVersionNumberRow, 2).Value)
Print #1, "#"
Print #1, "# Device Model: " + ws.Cells(SETTModelRow, 2).Value
Print #1, "# Network Name: " + ws.Cells(SETTNetworkNameRow, 2).Value
Print #1, "# LAN1 Address: " + ws.Cells(SETTLAN1AddressRow, 2).Value
Print #1, "# LAN2 Address: " + ws.Cells(SETTLAN2AddressRow, 2).Value
Print #1, "#"
Print #1, " "

If ws.Cells(SETTModelRow, 2).Value = "MOXA UC-7410" Then
    Print #1, "/root/moxa7gate_UC7410 \"
ElseIf ws.Cells(SETTModelRow, 2).Value = "MOXA UC-7410 Plus" Then
    Print #1, "/home/root/moxa7gate_UC7410PLUS \"
ElseIf ws.Cells(SETTModelRow, 2).Value = "MOXA UC-8410" Then
    Print #1, "/home/root/moxa7gate_UC8410 \"
ElseIf ws.Cells(SETTModelRow, 2).Value = "Cygwin" Then
    Print #1, "./moxa7gate_cygwin.exe \"
Else
    Print #1, "./moxa7gate_i386 \"
End If

' ����� ����������
str = "--Object " + """"
str = str + CStr(ws.Cells(SETTObjectRow, 2).Value) + """ \"
Print #1, str

str = "--Location " + """"
str = str + CStr(ws.Cells(SETTLocationRow, 2).Value) + """ \"
Print #1, str

str = "--Label " + """"
str = str + CStr(ws.Cells(SETTLabelRow, 2).Value) + """ \"
Print #1, str

str = "--NetworkName " + """"
str = str + CStr(ws.Cells(SETTNetworkNameRow, 2).Value) + """ \"
Print #1, str

str = "--LAN1Address " + """"
str = str + CStr(ws.Cells(SETTLAN1AddressRow, 2).Value) + """ \"
Print #1, str

str = "--LAN2Address " + """"
str = str + CStr(ws.Cells(SETTLAN2AddressRow, 2).Value) + """ \"
Print #1, str

str = "--VersionNumber " + """"
str = str + CStr(ws.Cells(SETTVersionNumberRow, 2).Value) + """ \"
Print #1, str

str = "--VersionTime " + """"
str = str + CStr(ws.Cells(SETTVersionTimeRow, 2).Value) + """ \"
Print #1, str

str = "--Model " + """"
str = str + CStr(ws.Cells(SETTModelRow, 2).Value) + """ \"
Print #1, str

' ��������� ������ ���������
If ws.Cells(SETTTCPPortRow, 2).Value <> "" Then
    Print #1, "--tcp_port " + CStr(ws.Cells(SETTTCPPortRow, 2).Value) + " \"
End If

'If ws.Cells(SETTMBAddressRow, 2).Value <> "" Then
'    Print #1, "--modbus_address " + CStr(ws.Cells(SETTMBAddressRow, 2).Value) + " \"
'End If

If ws.Cells(SETTStatusInfoBlockRow, 2).Value <> "" Then
    Print #1, "--status_info " + CStr(ws.Cells(SETTStatusInfoBlockRow, 2).Value) + " \"
End If

If ws.Cells(SETTShowSysMessagesRow, 2).Value = "��" Then
    Print #1, "--show_sys_messages \"
End If

If ws.Cells(SETTMap4Xto2XRow, 2).Value = "��" Then
    Print #1, "--map2Xto4X \"
End If

If ws.Cells(SETTMap3Xto4XRow, 2).Value = "��" Then
    Print #1, "--map3Xto4X \"
End If

' �����
If ws.Cells(SETTWatchdogTimerRow, 2).Value = "��" Then
    Print #1, "--watchdog_timer \"
End If

If ws.Cells(SETTShowDataFlowRow, 2).Value = "��" Then
    Print #1, "--show_data_flow \"
End If

If ws.Cells(SETTBuzzerRow, 2).Value = "��" Then
    Print #1, "--use_buzzer \"
End If

'**** ������ ���������� ������������ ���������������� ����������� ****

Set ws = Worksheets("���������������� ����������")
'ws.Activate

For i = 2 To 9

res = (ws.Cells(i, SLGWModeColumn).Value <> "") And (ws.Cells(i, SLGWModeColumn).Value <> "��������")

    If res Then

        str = "PORT" + Format(i - 1) + " "
        str = str + CStr(ws.Cells(i, SLModeColumn).Value) + " "
        str = str + CStr(ws.Cells(i, SLSpeedColumn).Value) + " "
        str = str + CStr(ws.Cells(i, SLParityColumn).Value) + " "
        str = str + CStr(ws.Cells(i, SLTimeoutColumn).Value) + " "
        str = str + CStr(ws.Cells(i, SLGWModeColumn).Value) + " "
        
        If ws.Cells(i, SLGWModeColumn).Value = "TCP_SERVER" Then
            str = str + CStr(ws.Cells(i, SLTCPPortColumn).Value) + " "
        End If
        
        If ws.Cells(i, SLCommentColumn).Value <> "" Then
            str = str + "--desc " + """"
            str = str + CStr(ws.Cells(i, SLCommentColumn).Value) + """ "
        End If
        
        Print #1, str + "\"
    End If

Next i

'**** ������ ���������� ������������ ���������� TCP-����������� ****

Set ws = Worksheets("Ethernet ����������")
'ws.Activate

For i = 2 To 33

res = (ws.Cells(i, LANTCPIPColumn).Value <> "")

    If res Then

        If i < 11 Then
          str = "TCP0" + Format(i - 1) + " "
        Else
          str = "TCP" + Format(i - 1) + " "
        End If

        str = str + CStr(ws.Cells(i, LANTCPIPColumn).Value) + ":"
        str = str + CStr(ws.Cells(i, LANTCPTCPColumn).Value) + " "
        str = str + CStr(ws.Cells(i, LANTCPUnitIDColumn).Value) + " "
        str = str + CStr(ws.Cells(i, LANTCPOffsetColumn).Value) + " "
        str = str + CStr(ws.Cells(i, LANTCPTimeoutColumn).Value) + " "
        
        If ws.Cells(i, LANTCPIP2Column).Value <> "" Then
          str = str + CStr(ws.Cells(i, LANTCPIP2Column).Value) + ":"
        Else
          str = str + "0.0.0.0:"
        End If
        
        If ws.Cells(i, LANTCPTCP2Column).Value <> "" Then
          str = str + CStr(ws.Cells(i, LANTCPTCP2Column).Value) + " "
        Else
          str = str + "502 "
        End If
        
        If ws.Cells(i, LANTCPCommentColumn).Value <> "" Then
            str = str + "--desc " + """"
            str = str + CStr(ws.Cells(i, LANTCPCommentColumn).Value) + """ "
        End If
        
        Print #1, str + "\"
    End If

Next i

'**** ������ ������� ���������� ������� ***

Set ws = Worksheets("����� �������")
'ws.Activate

For ATMIfaceColumn = ATMFirstColumn To 3 * ATMColumnsAmount - 1 Step 3
  
  ATMBusColumn = ATMIfaceColumn + 1
    
  ' ��������� ��������� ������� �� ������� �������� ����
  GoodColumn = False
  For ATMRow = ATMFirstRow To ATMFirstRow + ATMRowsAmount - 1
    If CStr(Mid(ws.Cells(ATMRow, ATMIfaceColumn).Value, 1, 1)) = "P" Or CStr(Mid(ws.Cells(ATMRow, ATMIfaceColumn).Value, 1, 1)) = "T" Or ws.Cells(ATMRow, ATMIfaceColumn).Value = "M7G" Then
      GoodColumn = True
    End If
  Next ATMRow

  If GoodColumn Then
    str = "AT" + CStr(CInt(ATMIfaceColumn \ 3 + 1)) + " "
    For ATMRow = ATMFirstRow To ATMFirstRow + ATMRowsAmount - 1
  
      IfaceNumber = 0
      If CStr(Mid(ws.Cells(ATMRow, ATMIfaceColumn).Value, 1, 1)) = "P" Then
        IfaceNumber = CInt(Mid(ws.Cells(ATMRow, ATMIfaceColumn).Value, 2, 1))
      ElseIf CStr(Mid(ws.Cells(ATMRow, ATMIfaceColumn).Value, 1, 1)) = "T" Then
        IfaceNumber = 10 * CInt(Mid(ws.Cells(ATMRow, ATMIfaceColumn).Value, 2, 1)) + CInt(Mid(ws.Cells(ATMRow, ATMIfaceColumn).Value, 3, 1)) + 16
      ElseIf ws.Cells(ATMRow, ATMIfaceColumn).Value = "M7G" Then
        IfaceNumber = 10 + 1
      End If
    
      If IfaceNumber <> 0 Then
        str = str + CStr(CInt(256 * (IfaceNumber - 1) + CByte(ws.Cells(ATMRow, ATMBusColumn).Value))) + " "
      Else
        str = str + "0 "
      End If
  
    Next ATMRow

  Print #1, str + "\"
  End If

Next ATMIfaceColumn

'**** ������ ������� ����������� ��������� ****

Set ws = Worksheets("����������� ����������")
'ws.Activate

For i = 2 To 129
    
    res = ws.Cells(i, RTMIfaceColumn).Value <> ""

    If res Then
        str = "RT "

        str = str + CStr(ws.Cells(i, RTMIfaceColumn).Value) + " "
        
        If ws.Cells(i, RTMMBAddressColumn).Value <> "" Then
          str = str + CStr(ws.Cells(i, RTMMBAddressColumn).Value) + " "
        Else
          str = str + "0 "
        End If
        
        str = str + CStr(ws.Cells(i, RTMMBTableColumn).Value) + " "
        str = str + CStr(ws.Cells(i, RTMOffsetColumn).Value) + " "
        str = str + CStr(ws.Cells(i, RTMStartColumn).Value) + " "
        str = str + CStr(ws.Cells(i, RTMLengthColumn).Value) + " "
        
        If ws.Cells(i, RTMCommentColumn).Value <> "" Then
            str = str + "--desc " + """"
            str = str + CStr(ws.Cells(i, RTMCommentColumn).Value) + """ "
        End If
        
        Print #1, str + "\"
    End If

Next i

'**** ������ ������� ������ ****

Set ws = Worksheets("������� ������")
'ws.Activate

For i = 2 To 129
    
    res = ws.Cells(i, PTIfaceColumn).Value <> ""

    If res Then
        str = "QT "
        
        str = str + CStr(ws.Cells(i, PTIfaceColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTMBAddressColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTMBTableColumn).Value) + " "
        
        If ws.Cells(i, PTAccessColumn).Value = "������/������" Then
          str = str + "RW "
        ElseIf ws.Cells(i, PTAccessColumn).Value = "������ ������" Then
          str = str + "W "
        ElseIf ws.Cells(i, PTAccessColumn).Value = "��������" Then
          str = str + "OFF "
        Else
          str = str + "R "
        End If
        
        str = str + CStr(ws.Cells(i, PTStartColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTLengthColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTOffsetColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTDelayColumn).Value) + " "
        str = str + CStr(ws.Cells(i, PTCriticalColumn).Value) + " "
        
        If ws.Cells(i, PTCommentColumn).Value <> "" Then
            str = str + "--desc " + """"
            str = str + CStr(ws.Cells(i, PTCommentColumn).Value) + """ "
        End If
        
        Print #1, str + "\"
    End If

Next i

'**** ������ ������� ���������� ****

Set ws = Worksheets("����������")
'ws.Activate

For i = 2 To 33

    res = ws.Cells(i, EXCPStageColumn).Value <> ""

    If res Then
      str = "EXPT "

      If CStr(ws.Cells(i, EXCPStageColumn).Value) = "������ ������� RAW" Then
      str = str + "QUERY_RECV_RAW "
      ElseIf CStr(ws.Cells(i, EXCPStageColumn).Value) = "������ �������" Then
      str = str + "QUERY_RECV "
      ElseIf CStr(ws.Cells(i, EXCPStageColumn).Value) = "���������������" Then
      str = str + "QUERY_FRWD "
      ElseIf CStr(ws.Cells(i, EXCPStageColumn).Value) = "����� �������" Then
      str = str + "RESPONSE_RECV "
      ElseIf CStr(ws.Cells(i, EXCPStageColumn).Value) = "�������� ������" Then
      str = str + "RESPONSE_SEND "
      Else ' ����� ������� RAW
      str = str + "RESPONSE_RECV_RAW "
      End If
      
      str = str + CStr(ws.Cells(i, EXCPActionColumn).Value) + " "
      
      If ws.Cells(i, EXCPPrm1Column).Value <> "" Then
        str = str + CStr(ws.Cells(i, EXCPPrm1Column).Value) + " "
      Else
        str = str + "0 "
      End If
      
      If ws.Cells(i, EXCPPrm2Column).Value <> "" Then
        str = str + CStr(ws.Cells(i, EXCPPrm2Column).Value) + " "
      Else
        str = str + "0 "
      End If
      
      If ws.Cells(i, EXCPPrm3Column).Value <> "" Then
        str = str + CStr(ws.Cells(i, EXCPPrm3Column).Value) + " "
      Else
        str = str + "0 "
      End If
      
      If ws.Cells(i, EXCPPrm4Column).Value <> "" Then
        str = str + CStr(ws.Cells(i, EXCPPrm4Column).Value) + " "
      Else
        str = str + "0 "
      End If
      
      If ws.Cells(i, EXCPCommentColumn).Value <> "" Then
        str = str + "--desc " + """"
        str = str + CStr(ws.Cells(i, EXCPCommentColumn).Value) + """ "
      End If
        
      Print #1, str + "\"
      End If

Next i

'**** ���������, ��������� ����

Print #1, "&"
Close #1

'Set ws = Worksheets("����")
'ws.Activate

End Sub

' ����� ������� ��� �������� ������������ ������������

Public Sub CheckSH()
  
  ErrorCount = 0
  Dim tmp As Integer
  
  tmp = ErrorCount
  Call CheckInterfaces
  If ErrorCount <> 0 Then
    MsgBox ("������� ������ � ����������: " + CStr(ErrorCount - tmp))
    End If
  
  tmp = ErrorCount
  Call CheckProxyTable
  If ErrorCount <> 0 Then
    MsgBox ("������� ������ � ����������: " + CStr(ErrorCount - tmp))
    End If
  
  End Sub

Private Sub CheckInterfaces()

Dim counter As Integer

Set ws = Worksheets("Serial ����������")
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
      ws.Cells(i, SLTCPPortColumn).AddComment ("�� ��������� ����� TCP-�����")
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
      ws.Cells(i, SLGWModeColumn).AddComment ("� ������� PROXY_TABLE ��� �������")
      ErrorCount = ErrorCount + 1
      Else
      ws.Cells(i, SLGWModeColumn).AddComment ("������� ") + CStr(counter) + " ������(��) � ������� PROXY_TABLE"
      End If
  
  Case "BRIDGE_PROXY"
  
  Case "BRIDGE_SIMPLE"
  
  Case Else
  
  End Select
  

Next i



End Sub

Private Sub CheckProxyTable()

Set ws = Worksheets("Serial ����������")
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
      ws2.Cells(j, PTPortColumn).AddComment ("��������� ���� ��������� �� � ������ GATEWAY_PROXY")
      ErrorCount = ErrorCount + 1
      End If
    
    End If

  Next j

End Sub

