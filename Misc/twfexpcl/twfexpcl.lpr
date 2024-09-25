program twfexpcl;

{$mode Delphi}{$H+}

uses
  Classes, SysUtils, twfdec, Interfaces;

{$R *.res}



procedure printMessage (msg: PChar; i : Integer); stdcall;
begin
  WriteLn (msg);
end;


procedure syntaxMessage ();
begin
  WriteLn ('Syntax: twfexpcl [-s][-b][-t <filetype>][-c <m1> ...] <twf file> <output path>');
  WriteLn ('');
  WriteLn (' -s           Memo files are exported as single files');
  WriteLn (' -b           OWBasic files use the extensions .bas, .app and .inc');
  WriteLn ('');
  WriteLn (' -t<filetype> specifies the export file type');
  WriteLn ('        filetype: 1 - TXT (default)');
  WriteLn ('                  2 - DDLink CSV');
  WriteLn ('                  3 - Excel CSV');
  WriteLn ('');
  WriteLn (' -c<m1>,...   specifies the modes to export. Multiple modes can be concatenated');
  WriteLn ('            mode: 1 - Schedule        4 - Business        7 - Memo');
  WriteLn ('                  2 - Todo            5 - Personal        8 - Sketch');
  WriteLn ('                  3 - Reminder        6 - Contacts        9 - Expense');
  WriteLn ('                  default: all');
  WriteLn ('');
  WriteLn ('Example call:');
  WriteLn (' twfexpcl -s -b -c 7 8 "C:\testfile.twf" "C:\output"');
  Write ('');
end;



var
  parcnt : Integer;
  n, cft, i, cp, e : Integer;
  str    : String;
  twfpath, writepath : String;
  twfflags, decflags : Integer;

begin;
  WriteLn ('twfexpcl v1.01 (c) 2006 by AUDACIA Software');
  WriteLn ('Based on TWF2CSV, (c) 1996-2003 by Zoltan Babos');
  WriteLn ('');

  parcnt := ParamCount ();
  twfflags := $01FF;
  decflags := 0;
  cft := 0;
  cp := 0;

  if parcnt = 0 then
  begin
    syntaxMessage ();
    exit;
  end;

  for n := 1 to parcnt do
    begin
    str := ParamStr (n);
    Val(str, i, e);

    if (cft <> 0) and (e = 0) then
    begin
      if cft = 1 then
      begin
        case i of
        1:  ; { TXT - default, do nothing }
        2:  decflags := decflags or decDDLinkCSV;
        3:  decflags := decflags or decExcelCSV;
        else
          begin
          WriteLn ('Error: Invalid parameter for -t');
          Halt (1);
          end;
        end;
      end
      else
        case i of
        1:  twfflags := twfflags or twfSchedule;
        2:  twfflags := twfflags or twfTodo;
        3:  twfflags := twfflags or twfReminder;
        4:  twfflags := twfflags or twfBusiness;
        5:  twfflags := twfflags or twfPersonal;
        6:  twfflags := twfflags or twfContacts;
        7:  twfflags := twfflags or twfMemo;
        8:  twfflags := twfflags or twfSketch;
        9:  twfflags := twfflags or twfExpense;
        else
          begin
          WriteLn ('Error: Invalid parameter for -c');
          Halt (1);
          end;
        end;
    end
    else if str = '-s' then
      decflags := decflags or decSingleMemo
    else if str = '-b' then
      decflags := decflags or decOWBExtensions
    else if str = '-t' then
      cft := 1
    else if str = '-c' then
    begin
      cft := 2;
      if twfflags = $01FF then
        twfflags := 0;
    end
    else
      begin
      case cp of
      0:
        begin
        twfpath := str;
        if ExtractFileExt (twfpath) = '' then
        twfpath := twfpath + '.twf';

        end;
      1:  writepath := str;
      else
        begin
        WriteLn ('Error: Too many parameters');
        Halt (1);
        end;
      end;
      inc (cp);
    end;
    end;

  decodeTWFFile (PChar (writepath), PChar (twfpath), twfflags, decflags,
                 printMessage, 0);

end.

