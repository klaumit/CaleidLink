object FrmSettings: TFrmSettings
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsDialog
  Caption = 'Settings'
  ClientHeight = 292
  ClientWidth = 310
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  KeyPreview = True
  OldCreateOrder = False
  Position = poMainFormCenter
  OnKeyDown = FormKeyDown
  DesignSize = (
    310
    292)
  PixelsPerInch = 96
  TextHeight = 13
  object BtnClose: TButton
    Left = 8
    Top = 259
    Width = 65
    Height = 25
    Anchors = [akLeft, akBottom]
    Caption = 'Close'
    TabOrder = 0
    OnClick = BtnCloseClick
  end
  object BtnOK: TButton
    Left = 236
    Top = 259
    Width = 65
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Default = True
    TabOrder = 1
    OnClick = BtnOKClick
  end
  object GpbConnection: TGroupBox
    Left = 8
    Top = 8
    Width = 293
    Height = 81
    Anchors = [akLeft, akTop, akRight]
    Caption = 'Connection'
    TabOrder = 2
    DesignSize = (
      293
      81)
    object LblDevice: TLabel
      Left = 14
      Top = 24
      Width = 36
      Height = 13
      Caption = 'Device:'
    end
    object LblSpeed: TLabel
      Left = 14
      Top = 51
      Width = 34
      Height = 13
      Caption = 'Speed:'
    end
    object CbxDevice: TComboBox
      Left = 104
      Top = 21
      Width = 173
      Height = 21
      Anchors = [akLeft, akTop, akRight]
      ItemHeight = 0
      TabOrder = 0
    end
    object CbxSpeed: TComboBox
      Left = 104
      Top = 48
      Width = 173
      Height = 21
      Style = csDropDownList
      Anchors = [akLeft, akTop, akRight]
      Enabled = False
      ItemHeight = 13
      ItemIndex = 2
      TabOrder = 1
      Text = '38400 bps'
      Items.Strings = (
        '9600 bps'
        '19200 bps'
        '38400 bps'
        '57600 bps'
        '115200 bps')
    end
  end
  object GpbUI: TGroupBox
    Left = 8
    Top = 95
    Width = 293
    Height = 146
    Anchors = [akLeft, akTop, akRight]
    Caption = 'User Interface'
    TabOrder = 3
    DesignSize = (
      293
      146)
    object LblLanguage: TLabel
      Left = 14
      Top = 77
      Width = 51
      Height = 13
      Caption = 'Language:'
    end
    object ChkShowBtnText: TCheckBox
      Left = 14
      Top = 24
      Width = 263
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Show button descriptions'
      Checked = True
      State = cbChecked
      TabOrder = 0
    end
    object ChkConfirmDelete: TCheckBox
      Left = 14
      Top = 47
      Width = 263
      Height = 17
      Anchors = [akLeft, akTop, akRight]
      Caption = 'Ask before deleting files'
      Checked = True
      State = cbChecked
      TabOrder = 1
    end
    object BtnResetScreenPosition: TButton
      Left = 14
      Top = 110
      Width = 263
      Height = 25
      Anchors = [akLeft, akRight, akBottom]
      Caption = 'Reset screen position'
      TabOrder = 2
      OnClick = BtnResetScreenPositionClick
    end
    object CbxLanguage: TComboBox
      Left = 104
      Top = 74
      Width = 173
      Height = 21
      Style = csDropDownList
      ItemHeight = 0
      TabOrder = 3
    end
  end
end
