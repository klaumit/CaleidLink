unit TWFDec;

{$mode Delphi}{$H+}

interface

uses
    SysUtils, Messages, Classes,
    Graphics, Controls,	StdCtrls, ExtCtrls;

type
  MessageCallbackProcedure = procedure (msg : PChar; i : Integer); stdcall;

const     // file flags
  twfSchedule = $0001;
  twfTodo     = $0002;
  twfReminder = $0004;
  twfBusiness = $0008;
  twfPersonal = $0010;
  twfContacts = $0020;
  twfMemo     = $0040;
  twfSketch   = $0080;
  twfExpense  = $0100;

const     // decode flags
  decTXT            = $0000;
  decDDLinkCSV      = $0001;
  decExcelCSV       = $0002;
  decOWBExtensions  = $0004;
  decSingleMemo     = $0008;

procedure decodeTWFFile (const writepath, twfpath : PChar; fileflags, decflags : Integer;
                         emsg : MessageCallbackProcedure; callbackVal : Integer); stdcall;

implementation



  const
	regfn:array[1..9] of string[15] =('Schedule','Reminder','To-do','Business',
																	 'Personal','Contact','Memo', 'Sketch', 'Expense');


{********************************************************************************}
{***************** C O N V E R T E R ********************************************}
{********************************************************************************}

procedure Masking(Bmp: TBitmap; BrushColor, MaskColor: TColor);
var
	Mask: TBitmap;
	Mask2: TBitmap;
begin
	Mask := TBitmap.Create;
	Mask.Assign(Bmp);
	Mask.Canvas.Brush.Color := MaskColor;
	Mask.Monochrome := True;

	if MaskColor=clwhite then
		try
		with bmp.Canvas do
			begin
			CopyMode := cmSrcAnd;
			Brush.Color := BrushColor;
			Font.Color := clWhite;
			Draw(0, 0, Mask);
			end;
		finally
			Mask.Free;
		end
	else
		begin
		Mask2 := TBitmap.Create;
		try
			Mask2.Assign(Bmp);
			with Mask2.Canvas do
			begin
				CopyMode := cmSrcAnd;
				Brush.Color := clBlack;
				Font.Color := clWhite;
				Draw(0, 0, Mask);
			end;
			with Bmp.Canvas do
			begin
				Brush.Color := BrushColor;
				{Fillrect(Rect(0,0,Bmp.Width, Bmp.height));}
				Fillrect(Cliprect);
				Brush.Color := clWhite;
				Font.Color := clBlack;
				CopyMode := cmSrcAnd;
				Draw(0, 0, Mask);
				CopyMode := cmSrcInvert;
				Draw(0, 0, Mask2);
			end;
		finally
			Mask.Free;
			Mask2.Free;
		end;
		end;
end;


function StrLLPos(const S2,S2e,S1: Pchar; const h1:word):Pchar;
var
h2,i,i1,j:word;
begin
	if (h1>0) and (S2+h1<=S2e) then   {S2<S2e helyett (S2+h1<=S2e) 971122}
		begin
		h2:=S2e-S2; i:=0;
		repeat
			while (i<h2) and (S1^<>S2[i]) do inc(i);    {(i<=h2) helyett < 1122}
			if h2<=i then
				begin
				result:=nil; exit;
				end;
			j:=1; inc(i); i1:=i;     {\/ i<=h2 helyett i1 971119  <= helyett < 1122}
			while (i1<h2) and (j<h1) and (S1[j]=S2[i1]) do
				begin
				inc(i1); inc(j);
				end;
		until j>=h1;
		result:=S2+i-1;
		end
	else
		result:=nil;
end;



function StrLLTran(S1:Pchar; var h1:word; const S2,S3: Pchar; const buff,h2,h3:word; const nulterm:boolean):word;
var
S1e:pchar;
begin
	result:=0;
	S1e:=S1+h1; {h1:=S1e-S1;}
	S1:=strllpos(S1,S1e,S2,h2);
	while S1<>nil do
		begin
		if h2<>h3 then
			begin
			if h3>h2 then
				begin
				if h1+(h3-h2)>buff then break;
				inc(h1, h3-h2);
				end
			else
				dec(h1, h2-h3);
			move((S1+h2)^,(S1+h3)^, S1e-S1-h2);
			dec(S1e,h2); inc(S1e,h3);
			end;
		move(S3^,S1^,h3);
		if h2<=h3 then inc(S1,h3);
		inc(result);
		S1:=strllpos(S1,S1e,S2,h2);
		end;
	if (h2<>h3) and nulterm then S1e[0]:=#0;
end;

procedure twfdecode(p:pchar; len:word);
var
	i:word;
	b:byte;
	p1,p2:integer;
begin
	for i:=1 to len do
		begin
		b:=byte(p^);
		p1:=(b div 32) +1;
		p1:=256-(p1*32);
		p2:=(b mod 32) div 8;
		p1:=p1+p2*8;
		p^:=char(p1+8-(b mod 8)-1);
		inc(p);
		end;
end;

procedure decodeTWFFile (const writepath, twfpath : PChar; fileflags, decflags : Integer;
                         emsg : MessageCallbackProcedure; callbackVal : Integer); stdcall;

  const
  	regnames:array[1..9,1..5] of pchar =(
  	(#10'Schedule-1'#10'Schedule-2'#10'Schedule-3',
  	 #8'Agenda-1'#8'Agenda-2'#8'Agenda-3',
  	 #12'Terminplan-1'#12'Terminplan-2'#12'Terminplan-3',
  	 #13'Rendez-vous-1'#13'Rendez-vous-2'#13'Rendez-vous-3',
  	 #15'Plan de citas-1'#15'Plan de citas-2'#15'Plan de citas-3'),
  	(#10'Reminder-1'#10'Reminder-2'#10'Reminder-3',
  	 #12'Promemoria-1'#12'Promemoria-2'#12'Promemoria-3',
  	 #12'Erinnerung-1'#12'Erinnerung-2'#12'Erinnerung-3',
  	 #14'Aide-mémoire-1'#14'Aide-mémoire-2'#14'Aide-mémoire-3',
  	 #10'Recuerdo-1'#10'Recuerdo-2'#10'Recuerdo-3'),
  	(#7'To-do-1'#7'To-do-2'#7'To-do-3',
  	 #10'Priorità-1'#10'Priorità-2'#10'Priorità-3',
  	 #8'Zu-Tun-1'#8'Zu-Tun-2'#8'Zu-Tun-3',
  	 #9'A faire-1'#9'A faire-2'#9'A faire-3',
  	 #12'Para-Hacer-1'#12'Para-Hacer-2'#12'Para-Hacer-3'),
  	(#15'Bus. Contacts-1'#15'Bus. Contacts-2'#15'Bus. Contacts-3',
  	 #17'Contatti lavoro-1'#17'Contatti lavoro-2'#17'Contatti lavoro-3',
  	 #21'Berufliche Kontakte-1'#21'Berufliche Kontakte-2'#21'Berufliche Kontakte-3',
  	 #25'Contacts professionnels-1'#25'Contacts professionnels-2'#25'Contacts professionnels-3',
  	 #19'contactos negocio-1'#19'contactos negocio-2'#19'contactos negocio-3'),
  	(#19'Personal Contacts-1'#19'Personal Contacts-2'#19'Personal Contacts-3',
  	 #18'Contatti privati-1'#18'Contatti privati-2'#18'Contatti privati-3',
  	 #18'Private Kontakte-1'#18'Private Kontakte-2'#18'Private Kontakte-3',
  	 #21'Contacts personnels-1'#21'Contacts personnels-2'#21'Contacts personnels-3',
  	 #20'contactos personal-1'#20'contactos personal-2'#20'contactos personal-3'),
  	(#10'Contacts-1'#10'Contacts-2'#10'Contacts-3',
  	 #10'Contatti-1'#10'Contatti-2'#10'Contatti-3',
  	 #10'Kontakte-1'#10'Kontakte-2'#10'Kontakte-3',
  	 #12'Releations-1'#12'Releations-2'#12'Releations-3',
  	 #11'Contactos-1'#11'Contactos-2'#11'Contactos-3'),
  	(#6'Memo-1'#6'Memo-2'#6'Memo-3',
  	 #9'Appunti-1'#9'Appunti-2'#9'Appunti-3',
  	 #6'Memo-1'#6'Memo-2'#6'Memo-3',
  	 #6'Memo-1'#6'Memo-2'#6'Memo-3',
  	 #6'Memo-1'#6'Memo-2'#6'Memo-3'),
  	(#8'Sketch-1'#8'Sketch-2'#8'Sketch-3',
  	 #9'Schizzo-1'#9'Schizzo-2'#9'Schizzo-3',
  	 #9'Skizzen-1'#9'Skizzen-2'#9'Skizzen-3',
  	 #9'Croquis-1'#9'Croquis-2'#9'Croquis-3',
  	 #9'Esbozos-1'#9'Esbozos-2'#9'Esbozos-3'),
  	(#9'Expense-1'#9'Expense-2'#9'Expense-3',
  	 #7'Spese-1'#7'Spese-2'#7'Spese-3',
  	 #10'Ausgaben-1'#10'Ausgaben-2'#10'Ausgaben-3',
  	 #10'Dépenses-1'#10'Dépenses-2'#10'Dépenses-3',
  	 #8'Gastos-1'#8'Gastos-2'#8'Gastos-3') 
  	);

	regn:array[1..9] of string[15] =('Schedule','Reminder','To-do','Business',
																	 'Personal','Contacts','Memo', 'Sketch', 'Expense');


    c_RemTipTxt: array[0..5] of string[14] =     {max 14 karakter!!!!!!!}
    	('Daily  [D]','Monthly 1 [M1]','Yearly 1  [Y1]','Weekly  [Wk]','Monthly 2 [M2]','Yearly 2  [Y2]');  

		{0              1                 2                 3           4                  5               }
var
	b1,i:byte;
	bmpdb,lang, converted:integer;
	fhandle,newhandle,bmphandle, Recpcs,Reci,len:longint;
	buf,p:pchar;
	len1,Recs,rh,Fldpcs,Flds:word;
	Rec,Recp:pchar;
	found, readed, anyfound :boolean;
	date1,date2,date3:TDatetime;
	time1,time2,time3:TDatetime;
	ntyp,rtyp,atyp,prior,cat:byte;
	s,amount,paym,exp:shortstring;
  ch : Char;
  filetitle : String;
  cptr : PAnsiChar;
	regfile : array[1..9] of String;


	function fileload:boolean;
	var
		Regtyp,Regtypp:integer;
		f:word;
	begin
	result:=false; found:=false; anyfound:=false;  readed:=false; bmpdb:=0; converted:=0;
	for Regtyp:=1 to 9 do if regfile[Regtyp]<>'' then
		try
		newhandle:=-1;
		Regtypp:=0;
		found:=false;
		repeat
			inc(Regtypp);
			try
				if not readed then
					begin
					len:=fileread(fhandle,buf^,65500);
					twfdecode(buf,len);
					end;
				readed:=false;
				if len=0 then
					begin
          emsg (PChar ('Error: ' + regn[Regtyp] + inttostr(Regtypp)
                     + ': Out of file (1)'), callbackVal);
          //WriteLn ('Error: ', regn[Regtyp], inttostr(Regtypp), ': Out of file (1)');
          Halt (1);
					//Memo1.lines.add('Error! '+regn[Regtyp]+inttostr(Regtypp)+': Out of file!  (1)');
					//exit;
					end;


				lang:=0;
				repeat
					inc(lang);
					rh:=strlen(regnames[Regtyp,lang]);
					p:=StrLLPos(buf,buf+len,regnames[Regtyp,lang],rh);
				until (p<>nil) or (lang>=high(regnames[Regtyp]));

				if p<>nil then
					begin
					found:=true; anyfound:=true;
					{if len-(p-buf)<21+2+21 then}
					fileseek(fhandle,(p-buf)-len,1);
					len:=fileread(fhandle,buf^,65500);
					twfdecode(buf,len);
					if len=0 then
						begin
            emsg (PChar ('Error: ' + regn[Regtyp] + inttostr(Regtypp)
                       + ': Out of file (2)'), callbackVal);
            //WriteLn ('Error: ', regn[Regtyp], inttostr(Regtypp), ': Out of file (2)');
            Halt (1);
						//Memo1.lines.add('Error! '+regn[Regtyp]+inttostr(Regtypp)+': Out of file!  (2)');
						//exit;
						end;
					p:=buf+rh;
					if (len>=rh+2+rh) and (strlcomp(p+2, regnames[Regtyp,lang],rh)=0) then
						inc(p,2+rh);
					inc(p,18); {Go to Record pcs number}
					Recpcs:=pword(p)^;
					inc(p,2+4); {Record pcs+ something #255 + #255 + #1 + #0}
					len1:=pword(p)^;  {CYCMemoItem or other text lenght}
					inc(p,2+len1);  {skip the CYCMemoItem or other text}
					Reci:=1;
					while (Reci<=Recpcs) do
						begin
						inc(p,20);{skip the "          $         "}
						Fldpcs:=1;
						case Regtyp of
							1,2:begin {'Schedule','Reminder'}
									try
										date1:=encodedate(pword(p)^, byte(p[2]),byte(p[3]));
									except
										date1:=encodedate(1980, 01,01);
									end;
									inc(p,4);
									inc(p); {Ascii 1}
									try
										time1:=encodetime(byte(p[0]),byte(p[1]),0,0);
									except
										time1:=0;
									end;
									inc(p,2);
									end;
								3:begin {'To-do'}
									try
										date2:=encodedate(pword(p)^, byte(p[2]),byte(p[3]));
									except
										date2:=encodedate(1980, 01,01);
									end;
									inc(p,4);
									inc(p); {Ascii 1}
									inc(p,2);
									end;
						 4..6:begin {'Business','Personal','Contacts'}
									Fldpcs:=pword(p)^; inc(p,2);
									end;
							 9: begin {'Expense'}
									try
										date1:=encodedate(pword(p)^, byte(p[2]),byte(p[3]));
									except
										date1:=encodedate(1980, 01,01);
									end;
									inc(p,4);
									i:=1;
									while (i<=10) and (p[10-i]<>#0) do
										begin amount[i]:=p[10-i]; inc(i); end;
									dec(i);
									if (i>0) and (amount[i]='.') then dec(i);
									setlength(amount,i);
									i:=pos('.',amount);
									if i>0 then amount[i]:=Decimalseparator;

									inc(p,11);

									i:=1;
									while (i<=14) and (p[14-i]<>#0) do
										begin exp[i]:=p[14-i]; inc(i); end;
									setlength(exp,i-1);
									inc(p,15);

									i:=1;
									while (i<=14) and (p[14-i]<>#0) do
										begin paym[i]:=p[14-i]; inc(i); end;
									setlength(paym,i-1);
									inc(p,15);

									inc(p,3); {something}
									end;
							else {'Memo','Sketch'};
							end;

						Recp:=Rec; Recs:=0;
						for f:=1 to Fldpcs do
							begin
							if byte(p^)<255 then {short Record <=255}
								begin
								Flds:=byte(p^);
								if Flds>0 then
									move(p[1],Recp^,Flds);
								inc(p,1+Flds);
								end
							else  {big record}
								begin
								inc(p);
								Flds:=pword(p)^;
								if Flds>0 then
									move(p[2],Recp^,Flds);
								inc(p,2+Flds);
								end;
							inc(Recp,Flds); Recp^:=#1; inc(Recp);
							inc(Recs,Flds+1);
							end;

						case Regtyp of
							1:  begin {54 pcs char in end of Schedule Rec}
									ntyp:=byte(p^); inc(p);
									inc(p,8); {something}
									try
										time2:=encodetime(byte(p[0]),byte(p[1]),0,0);
									except
										time2:=0;
									end;
									inc(p,2);
									inc(p); {Ascii 1}
									try
										time3:=encodetime(byte(p[0]),byte(p[1]),0,0);
									except
										time3:=0;
									end;
									inc(p,2);
									try
										date2:=encodedate(pword(p)^, byte(p[2]),byte(p[3]));
									except
										date2:=encodedate(1980, 01,01);
									end;
									inc(p,4);

									s:=datetostr(date1)+#1;
									case ntyp of
										1:s:=s+#1+#1+#1+#1;
										5:s:=s+#1+timetostr(time1)+#1+#1+#1;
										13:s:=s+#1+timetostr(time1)+#1+timetostr(time2)+#1+#1;
										21:s:=s+#1+timetostr(time1)+#1+#1+timetostr(time3)+#1;
										29:s:=s+#1+timetostr(time1)+#1+timetostr(time2)+#1+timetostr(time3)+#1;
										35:s:=s+datetostr(date2)+#1+#1+#1+#1;
										else
											 s:=s+datetostr(date2)+#1+timetostr(time1)+#1+timetostr(time2)+#1+timetostr(time3)+#1;
										end;
									Flds:=length(s);
									move(Rec^,Rec[Flds],Recs); inc(Recs,Flds);
									move(s[1],Rec^,Flds);
									inc(p, 30); {something}
									inc(p,6); {something chars in end of Rec}
									end;
							2:  begin {18 pcs char in end of Reminder Rec}
									ntyp:=byte(p^); inc(p);
									inc(p,8); {something}
									try
										time2:=encodetime(byte(p[0]),byte(p[1]),0,0);
									except
										time2:=0;
									end;
									inc(p,2);
									inc(p); {Ascii 1}
									try
										time3:=encodetime(byte(p[0]),byte(p[1]),0,0);
									except
										time3:=0;
									end;
									inc(p,2);

									rtyp:=byte(p^); inc(p);

									s:=datetostr(date1)+#1;
									case ntyp of
										1:s:=s+#1+#1+#1+#1;
										5:s:=s+#1+timetostr(time1)+#1+#1+#1;
										13:s:=s+#1+timetostr(time1)+#1+timetostr(time2)+#1+#1;
										21:s:=s+#1+timetostr(time1)+#1+#1+timetostr(time3)+#1;
										29:s:=s+#1+timetostr(time1)+#1+timetostr(time2)+#1+timetostr(time3)+#1;
										else
											 s:=s+#1+#1+timetostr(time1)+#1+timetostr(time2)+#1+timetostr(time3)+#1;
										end;

{	('Daily  [D]','Monthly 1 [M1]','Yearly 1  [Y1]','Weekly  [Wk]','Monthly 2 [M2]','Yearly 2  [Y2]');
		0              1                 2                 3           4                  5               }

									case rtyp of
										0:b1:=0;
										1:b1:=3;
										2:b1:=1;
										3:b1:=4;
										4:b1:=2;
										5:b1:=5;
										end;

									s:=s+c_RemTipTxt[b1]+#1;

									if (decflags and decDDLinkCSV) <> 0 then
										begin
										s:=s+#1+#1+#1+#1;
										end;

									Flds:=length(s);
									move(Rec^,Rec[Flds],Recs); inc(Recs,Flds);
									move(s[1],Rec^,Flds);
									inc(p, 3); {something}
									end;

							3:  begin {36 pcs char in end of Todo}
									ntyp:=byte(p^); inc(p); {checked=1  not checked=0}
									atyp:=byte(p^); inc(p); {with Alarm =26  not alarm=2}
									inc(p,7); {something}
									inc(p,3); {something}
									try
										date1:=encodedate(pword(p)^, byte(p[2]),byte(p[3]));
									except
										date1:=encodedate(1980, 01,01);
									end;
									inc(p,4);
									inc(p,4); {something}
									try
										time3:=encodetime(byte(p[0]),byte(p[1]),0,0);
									except
										time3:=0;
									end;
									inc(p,2);

									prior:=byte(p^);
									inc(p);

									inc(p,5); {something}

									try
										date3:=encodedate(pword(p)^, byte(p[2]),byte(p[3]));
									except
										date3:=encodedate(1980, 01,01);
									end;
									inc(p,4);

									cat:=byte(p^);
									inc(p);

									if ntyp=1 then
										s:='×'#1+datetostr(date2)+#1
									else
										s:=#1#1;

									case prior of
										$10:s:=s+'A'#1;
										$20:s:=s+'B'#1;
										$30:s:=s+'C'#1;
										end;

									case cat of
										$0:s:=s+'Business'#1;
										$1:s:=s+'Personal'#1;
										$2:s:=s+'Free'#1;
										end;

									s:=s+datetostr(date1)+#1;
									if atyp=26 then
										s:=s+datetostr(date3)+#1+timetostr(time3)+#1
									else
										s:=s+#1#1;

									Flds:=length(s);
									move(Rec^,Rec[Flds],Recs); inc(Recs,Flds);
									move(s[1],Rec^,Flds);

									inc(p,3); {something}
									end;


							9: begin {119 pcs char in end of Expense Rec}
								 s:=datetostr(date1)+#1+amount+#1+paym+#1+exp+#1;
								 Flds:=length(s);
								 move(Rec^,Rec[Flds],Recs); inc(Recs,Flds);
								 move(s[1],Rec^,Flds);
								 inc(p,119); {something}
								 end;

							8: begin {Sketch}
								 inc(p,54); {something}
								 Flds:=pword(p)^;
								 inc(p,4);
								 inc(bmpdb);
								 bmphandle:=filecreate(changefileext(regfile[Regtyp],'')+inttostr(bmpdb)+'.bmp');
								 filewrite(bmphandle, p^, Flds);
								 fileclose(bmphandle);
								 inc(p,Flds+4);
								 s:=regfile[Regtyp]+inttostr(bmpdb)+'.bmp'#1;
								 Flds:=length(s);
								 move(s[1],Rec[Recs],Flds); inc(Recs,Flds);
								 end;
							else {Memo, Business',Personal,Contacts}
									inc(p,6); {something chars in end of Rec}
							end;


						while (Recs>0) and (Rec[Recs-1]=#1) do dec(Recs);

						if Recs>0 then {Saving}
							begin
							if (decflags and decDDLinkCSV) <> 0 then  {DD-Link CSV}
								begin
								StrLLTran(Rec,Recs,';', ',', 65500, 1,1, false);
								StrLLTran(Rec,Recs,#1, ';', 65500, 1,1, false);
								move(Rec^,Rec[1],Recs); Rec^:=';'; inc(Recs);
								StrLLTran(Rec,Recs,#13,'__', 65500, 1,2, false)
								end
							else if (decflags and decExcelCSV) <> 0 then {EXCEL CSV}
								begin
								StrLLTran(Rec,Recs,';', ',', 65500, 1,1, false);
								StrLLTran(Rec,Recs,#1, ';', 65500, 1,1, false);
								StrLLTran(Rec,Recs,'"'#13, '" '#13, 65500, 2,3, false);
								StrLLTran(Rec,Recs,#13,#13#10, 65500, 1,2, false);
								move(Rec^,Rec[1],Recs);
								Rec^:='"'; inc(Recs);
								Rec[Recs]:='"'; inc(Recs);
								end
							else   {WORD TXT}
								begin
								StrLLTran(Rec,Recs,#1, #9, 65500, 1,1, false);
								StrLLTran(Rec,Recs,#13,#13#10, 65500, 1,2, false);
								Rec[Recs]:=#13; inc(Recs);
								Rec[Recs]:=#10; inc(Recs);
								end;

							Rec[Recs]:=#13; inc(Recs);
							Rec[Recs]:=#10; inc(Recs);


              if (Regtyp = 7) and ((decflags and decSingleMemo) <> 0) then  // save single memos
                begin;
                filetitle := '';
                cptr := Rec;
                ch := #0;
                while (cptr^ <> #13) and (cptr^ <> #10) do
                  begin
                  if (cptr = Rec) and (cptr^ = '!') and ((decflags and decOWBExtensions) <> 0) then
                    begin;
                    inc (cptr);
                    ch := cptr^;
                    inc (cptr);
                    end;
                  filetitle := filetitle + cptr^;
                  inc (cptr);
                  end;

                if (ch <> #0) and ((decflags and decOWBExtensions) <> 0) then
                begin
                  case ch of
                  ' ':  filetitle := filetitle + '.bas';
                  '+':  filetitle := filetitle + '.app';
                  '*':  filetitle := filetitle + '.inc';
                  else  filetitle := '!' + ch + filetitle; // restore original filename
                  end;
                end;

                if ExtractFileExt (filetitle) = '' then
                  filetitle := filetitle + '.txt'; // default extension

                if not DirectoryExists (writepath + '\'+ IntToStr (Regtypp)) then
                  CreateDir (PChar (writepath + '\'+ IntToStr (Regtypp)) );

                newhandle := FileCreate (writepath + '\' + IntToStr (Regtypp) + '\' + filetitle);
                FileWrite (newhandle, Rec^, Recs);
                FileClose (newhandle);
                newhandle := 0;
                end
              else
                begin
  							if newhandle<0 then
  								if not (Regtyp in [6,7]) then
  									newhandle:=filecreate(regfile[Regtyp])
  								else
  									newhandle:=filecreate(changefileext(regfile[Regtyp],'')+'_'+inttostr(Regtypp)+extractfileext(regfile[Regtyp]));
  							filewrite(newhandle, Rec^,Recs);
                end;
							end
						else
							begin
              emsg (PChar ('Error: ' + regn[Regtyp] + inttostr(Regtypp)
                         + ': Record length is zero'), callbackVal);
              //WriteLn ('Error: ', regn[Regtyp], inttostr(Regtypp), ': Record length is zero');
							//Memo1.lines.add('Error! '+regn[Regtyp]+inttostr(Regtypp)+': The record length is zero!');
							//application.processmessages;
							break;
							end;

						if (len=65500) and ((len-(p-buf))<5000) then
							begin
							fileseek(fhandle,(p-buf)-len,1);
							len:=fileread(fhandle,buf^,65500);
							twfdecode(buf,len);
							if len=0 then
								begin
                emsg (PChar ('Error: ' + regn[Regtyp] + inttostr(Regtypp)
                           + ': Out of file (3)'), callbackVal);
                //WriteLn ('Error: ', regn[Regtyp], inttostr(Regtypp), ': Out of file (3)');
                Halt (1);
								//Memo1.lines.add('Error! '+regn[Regtyp]+inttostr(Regtypp)+': Out of file!  (3)');
								//exit;
								end;
							p:=buf;
							end;

						inc(Reci);
						end;

					if (Reci-1>0) then
						begin
            emsg (PChar (regn[Regtyp] + inttostr(Regtypp) + ': '
                       + IntToStr (Reci - 1) + ' record(s) converted.'), callbackVal);
            //WriteLn (regn[Regtyp], inttostr(Regtypp), ': ', inttostr(Reci-1), 'record(s) converted.');
						//Memo1.lines.add(regn[Regtyp]+inttostr(Regtypp)+': '+inttostr(Reci-1)+' record(s) converted.');
						inc(converted);
						//application.processmessages;
						end;

					fileseek(fhandle,(p-buf)-len,1);
					end
				else if found then
					begin readed:=true; break end
				else
					begin
          emsg (PChar ('Error: ' + regn[Regtyp] + inttostr(Regtypp)
                     + ': Register not found'), callbackVal);
          //WriteLn ('Error: ', regn[Regtyp], inttostr(Regtypp), ': Register not found');
					//Memo1.lines.add('Error! '+regn[Regtyp]+inttostr(Regtypp)+': Register not found!');
					fileseek(fhandle,0,0);
					break;
					end;
			finally
				if (Regtyp in [6,7]) and (newhandle>=0) then
					begin fileclose(newhandle); newhandle:=-1 end
			end;
		until false;


		finally
			if not (Regtyp in [6,7]) and (newhandle>=0) then
				begin fileclose(newhandle); newhandle:=-1 end
		end;

	if converted=0 then
		begin
		if anyfound then
      emsg ('Warning: No data item for conversion', callbackVal)
      //WriteLn ('Warning: No data item for conversion')
			//Memo1.lines.add('Warning! There is no data item for conversion!')
		else
      emsg ('Error! This software not yet support this TWF file! Please, send this TWF file to ddlink@freemail.hu E-mail address for examination!', callbackVal);
      //WriteLn ('Error! This software not yet support this TWF file! Please, send this TWF file to ddlink@freemail.hu E-mail address for examination!');
			//Memo1.lines.add('Error! This software not yet support this TWF file! Please, send this TWF file to ddlink@freemail.hu E-mail address for examination!')
		end
	else
    emsg ('Done.', callbackVal);
    //WriteLn ('Done.');
		//Memo1.lines.add('done.');

	result:=true;
	end;

begin;
	//Memo1.clear;

  if (fileflags and twfSchedule) <> 0 then
    regfile[1] := writepath + '\Schedule.txt';
  if (fileflags and twfReminder) <> 0 then
    regfile[2] := writepath + '\Reminder.txt';
  if (fileflags and twfTodo) <> 0 then
    regfile[3] := writepath + '\Todo.txt';
  if (fileflags and twfBusiness) <> 0 then
    regfile[4] := writepath + '\Business.txt';
  if (fileflags and twfPersonal) <> 0 then
    regfile[5] := writepath + '\Personal.txt';
  if (fileflags and twfContacts) <> 0 then
    regfile[6] := writepath + '\Contacts.txt';
  if (fileflags and twfMemo) <> 0 then
    regfile[7] := writepath + '\Memo.txt';
  if (fileflags and twfSketch) <> 0 then
    regfile[8] := writepath + '\Sketch.txt';
  if (fileflags and twfExpense) <> 0 then
    regfile[9] := writepath + '\Expense.txt';

	buf:=stralloc(65500);
	Rec:=stralloc(65500);
	try
		fhandle:=fileopen(twfpath, fmShareDenyNone);
		if fhandle>=0 then
			try
				if not fileload then exit;
			finally
				fileclose(fhandle);
			end
		else
      emsg (PChar ('Error:  File not found (' + twfpath + ')'), callbackVal);
      //WriteLn ('Error:  File not found (', twfpath, ')');
			//Memo1.lines.add('Error!  File not found! ('+CBTWF.Text+')');

	finally
		strdispose(buf);
		strdispose(Rec);
	end;
end;
end.
