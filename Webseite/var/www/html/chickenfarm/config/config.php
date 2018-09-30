<html>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge">
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<meta name="robots" content="INDEX,FOLLOW">
<meta name="page-type" content=" homepage">
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta name="keywords"
content="homepage,dokument,webpage,page,web,netz,homepage dokument webpage page web netz">
<meta name="description"
content="homepage, dokument, webpage, page, web, netz" >
<title>Chickenfarm</title>

<!-- /////////////////// Copyright-Vermerk /////////////////// -->
<!-- Der nachfolgende einzeilige Copyright-Vermerk (c) ist nicht zu l�schen.-->
<!-- (c)Copyright by S.I.S.Papenburg / www.on-mouseover.de/templates/ -->
<!--Ein Entfernen dieses Copyright/Urheberrecht-Vermerks kann rechtliche Schritte nach sich ziehen -->
<!-- ////////////////////////////////////// -->
<link href="../../images/chicken.ico" rel="shortcut Icon"/>
<link rel="stylesheet" href="../css/menue.css" type="text/css">
<link rel="stylesheet" href="../css/format.css" type="text/css">
<link rel="stylesheet" href="../css/animation.css" type="text/css">
<link rel="stylesheet" href="../css/font-awesome.min.css" type="text/css">
<link rel="stylesheet" href="../css/font-roboto-light.css" type="text/css">
<link rel="stylesheet" href="../css/font-open-sans-light.css" type="text/css">


<!-- /////////////////// BROWSERWEICHE  - Conditional Comments f�r Internet Explorer Vers. 8 und tiefer /////////////////// -->
<!-- ========== alte IE-Versionen unterhalb Version 9 - lt=lower than IE9 ========== -->
<!--[if lt IE 9]>
   <link href="css/hinweis-alte-ie.css" rel="stylesheet" type="text/css"/>
<![endif]-->

<!--Javascript Implementierung -->
<script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
<script src="http://ajax.googleapis.com/ajax/libs/jquery/1.10.2/jquery.min.js" type="text/javascript"></script>
<script type="text/javascript">

//--Textfile for runtime  -->
$(document).ready(function(){
	$('#runtime').load('../../tmp/runtime.txt'); ///var/www/html/tmp/runtime.txt
});
</script>
</head>
<body>
<div class="container_haupt" id="top">
<div class="wrapper">

<!-- ============================== HAUPTBEREICH ============================== -->

<!-- ############################################################ -->
<!-- kopf bereich -->
<!-- ############################################################ -->

<div class="kopf-main">
<div class="kopf">

<div class="kopfbox-1a"><span class="name"> Chickenfarm</span></div>
<div class="kopfbox-1b icon-liste">
<ul>
<li><a href="index.html" title="Social Media"><i class="fa fa-facebook drehen" ></i></a></li>
<li><a href="index.html" title="Social Media"><i class="fa fa-twitter drehen"></i></a></li>
<li><a href="index.html" title="Social Media"><i class="fa fa-youtube drehen" ></i></a></li>
<li><a href="index.html" title="Social Media"><i class="fa fa-instagram drehen" ></i></a></li>
</ul>
</div>

</div>
</div>
<!-- ############################################################ -->
<!-- logo bereich -->
<!-- ############################################################ -->

<div class="logo-main">
<div class="logo">

<div class="logobox-1a"></div>
<div class="logobox-1b">
<!-- = = = = = nav bereich = = = = =  -->
<input type="checkbox" id="checkbox_toggle">
<span class="seitentitel"><i class="fa fa-file-o" ></i> Konfiguration</span><label for="checkbox_toggle"><i class="fa fa-navicon drehen" ></i> Men&uuml;</label>
<!-- Men� -->
<div id="menu1">
<ul>
<li><a  href="../index.html">Home</a></li>
<li><a  href="../bewohner.html">Bewohner</a></li>
<li><a  href="../webcam.html">Webcam</a></li>
<li id="aktuell"><a  href="config.php">Konfiguration</a></li>
<li><a  href="../../../index.php">Pi Control</a></li>
<!-- wird aktuell nicht ben�tigt
<li><a  href="index4.html">Quartus</a></li>
<li><a  href="index5.html">Quintus</a></li>
<li><a  href="index6.html">Sextus</a></li>
<li><a  href="index7.html">Septus</a></li>
 -->
</ul>
</div>
<!-- Men� ende  -->
<!-- = = = = = nav bereich ende = = = = = -->
</div>

</div>
</div>
<!-- ############################################################ -->
<!-- content bereich -->
<!-- ############################################################ -->

<div class="content-main">
<div class="content">

<!-- spalte 1 -->
<div class="contentbox-1a">

<h1>Konfiguration</h1><hr>
<h2>Helligkeit</h2>
Der aktuelle Grenzwert betr&auml;gt:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('LUXthreshold.txt');
	echo "$datei_inhalt LUX";
?>
<!--Save new Value in file-->
<form action="config.php" method="post">
<p>Neuer Wert:<input type="Text" name="LUXthresholdInputField">
<input type="submit" name="LUXthresholdInput" value="Speichern"></p>
</form>

<?php
//if the Variable $_POST ['form1'] was defined and the formulr sent the action will be done
if(isset($_POST['LUXthresholdInput'])){
	//Check if input field was empty
	if(empty($_POST['LUXthresholdInputField']) == TRUE ){
		echo "<p><font color='#FF0000'>Upload failed. Empty Cell</font></p>";
	}
	else{
		//Check if only numbers bill be used
		if (ctype_digit($_POST['LUXthresholdInputField'])){
			//Dateinamen festlegen
			$datei_name = 'LUXthreshold.txt';
			//Inhalt
			$inhalt = $_POST['LUXthresholdInputField'];
			//Datei wird zum schreiben geoeffnet
			$fp = fopen ($datei_name, w);
			//schreibe den Inhalt von Value
			fwrite ($fp, $_POST['LUXthresholdInputField']);
			//Datei schliesen
			fclose ($fp);
			echo "<p><font color='#008000'>Upload successful: " . $dir ."/". $datei_name ."</p>Wert $inhalt in Datei geschrieben</font></p>";
		}
		else{
			echo "<p><font color='#FF0000'>Only numbers are accepted.</font></p>";
		}
	}
}
?>
Der aktuelle Hysterese betr&auml;gt:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('LUXhysteresis.txt');
	echo "$datei_inhalt LUX";
?>
<!--Save new Value in file-->
<form action="config.php" method="post">
<p>Neuer Wert:<input type="Text" name="LUXhysteresisInputField">
<input type="submit" name="LUXhysteresisInput" value="Speichern"></p>
</form>
<?php
//if the Variable $_POST ['form1'] was defined and the formulr sent the action will be done
if(isset($_POST['LUXhysteresisInput'])){
	//Check if input field was empty
	if(empty($_POST['LUXhysteresisInputField']) == TRUE ){
	echo "<p><font color='#FF0000'>Upload failed. Empty Cell</font></p>";
	}
	else{
		//Check if only numbers bill be used
		if (ctype_digit($_POST['LUXhysteresisInputField'])){
			//Dateinamen festlegen
			$datei_name = 'LUXhysteresis.txt';
			//Inhalt
			$inhalt = $_POST['LUXhysteresisInputField'];
			//Datei wird zum schreiben geoeffnet
			$fp = fopen ($datei_name, "w" );
			//schreibe den Inhalt von Value
			fwrite ( $fp, $_POST['LUXhysteresisInputField'] );
			//Datei schliesen
			fclose ( $fp );
			echo "<p><font color='#008000'>Upload successful: " . $dir ."/". $datei_name ."</p>Wert $inhalt in Datei geschrieben</p></font>";
		}
		else{
			echo "<p><font color='#FF0000'>Only numbers are accepted.</font></p>";
		}
	}
}
?><hr>
<h2>Temperatur</h2>
Der aktuelle Grenzwert betr&auml;gt:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('TEMPthreshold.txt');
	echo "$datei_inhalt &deg;C";
?>
<!--Save new Value in file-->
<form action="config.php" method="post">
<p>Neuer Wert:<input type="Text" name="TEMPthresholdInputField">
<input type="submit" name="TEMPthresholdInput" value="Speichern"></p>
</form>
<?php
//if the Variable $_POST ['form1'] was defined and the formulr sent the action will be done
if(isset($_POST['TEMPthresholdInput'])){
	//Check if input field was empty
	if(empty($_POST['TEMPthresholdInputField']) == TRUE ){
	echo "<p><font color='#FF0000'>Upload failed. Empty Cell</font></p>";
	}
	else{
		//Check if only numbers bill be used
		if (ctype_digit($_POST['TEMPthresholdInputField'])){
			//Dateinamen festlegen
			$datei_name = 'TEMPthreshold.txt';
			//Inhalt
			$inhalt = $_POST['TEMPthresholdInputField'];
			//Datei wird zum schreiben geoeffnet
			$fp = fopen ($datei_name, "w" );
			//schreibe den Inhalt von Value
			fwrite ( $fp, $_POST['TEMPthresholdInputField'] );
			//Datei schliesen
			fclose ( $fp );
			echo "<p><font color='#008000'>Upload successful: " . $dir ."/". $datei_name ."</p>Wert $inhalt in Datei geschrieben</p></font></p>";
		}
		else{
			echo "<p><font color='#FF0000'>Only numbers are accepted.</font></p>";
		}
	}
}
?>
Der aktuelle Hysterese betr&auml;gt:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('TEMPhysteresis.txt');
	echo "$datei_inhalt &deg;C";
?>
<!--Save new Value in file-->
<form action="config.php" method="post">
<p>Neuer Wert:<input type="Text" name="TEMPhysteresisInputField">
<input type="submit" name="TEMPhysteresisInput" value="Speichern"></p>
</form>
<?php
//if the Variable $_POST ['form1'] was defined and the formulr sent the action will be done
if(isset($_POST['TEMPhysteresisInput'])){
	//Check if input field was empty
	if(empty($_POST['TEMPhysteresisInputField']) == TRUE ){
	echo "<p><font color='#FF0000'>Upload failed. Empty Cell</font></p>";
	}
	else{
		//Check if only numbers bill be used
		if (ctype_digit($_POST['TEMPhysteresisInputField'])){
			//Dateinamen festlegen
			$datei_name = 'TEMPhysteresis.txt';
			//Inhalt
			$inhalt = $_POST['TEMPhysteresisInputField'];
			//Datei wird zum schreiben geoeffnet
			$fp = fopen ($datei_name, "w" );
			//schreibe den Inhalt von Value
			fwrite ( $fp, $_POST['TEMPhysteresisInputField'] );
			//Datei schliesen
			fclose ( $fp );
			echo "<p><font color='#008000'>Upload successful: " . $dir ."/". $datei_name ."</p>Wert $inhalt in Datei geschrieben</font></p>";
		}
		else{
			echo "<p><font color='#FF0000'>Only numbers are accepted.</font></p>";
		}
	}
}
?><hr>
<h2>Tor</h2>
Modus (0 = Manual Modus | 1 = Automatic Modus)<br>
aktueller Modus:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('Door_Modus.txt');
	echo "$datei_inhalt";
?>
<br><br>
Neuer Modus festlegen:<br>
<!--Save new Value in file-->
<form action="config.php" method="post">

	<input type="radio" name="Door_ModusField" value="0"> Manual Modus<br>
  <input type="radio" name="Door_ModusField" value="1" checked="checked"> Automatic Modus<br>

	<input type="submit" name="Door_ModusInput" value="Speichern"></p>
</form>
<?php
//if the Variable $_POST ['form1'] was defined and the formulr sent the action will be done
if(isset($_POST['Door_ModusField'])){
	//Dateinamen festlegen
	$datei_name = 'Door_Modus.txt';
	//Inhalt
	$inhalt = $_POST['Door_ModusField'];
	//Datei wird zum schreiben geoeffnet
	$fp = fopen ($datei_name, "w" );
	//schreibe den Inhalt von Value
	fwrite ( $fp, $_POST['Door_ModusField'] );
	//Datei schliesen
	fclose ( $fp );
	echo "<p><font color='#008000'>Upload successful: " . $dir ."/". $datei_name ."</p>Wert $inhalt in Datei geschrieben</p></font></p>";
}
?>
<br>Fahrzeit bestimmen.<br>
aktuelle Zeit:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('DOORMOVETIME.txt');
	echo "$datei_inhalt Sekunden";
?>
<br>
<!--Save new Value in file-->
<form action="config.php" method="post">
<p>Neuer Zeitwert:<input type="Text" name="Door_TimeField">(in Sekunden)
<input type="submit" name="Door_TimeFieldInput" value="Speichern"></p>
</form>
<?php
//if the Variable $_POST ['form1'] was defined and the formulr sent the action will be done
if(isset($_POST['Door_TimeFieldInput'])){
	//Check if input field was empty
	if(empty($_POST['Door_TimeField']) == TRUE ){
	echo "<p><font color='#FF0000'>Upload failed. Empty Cell</font></p>";
	}
	else{
		//Check if only numbers bill be used
		if (ctype_digit($_POST['Door_TimeField'])){
			//Dateinamen festlegen
			$datei_name = 'DOORMOVETIME.txt';
			//Inhalt
			$inhalt = $_POST['Door_TimeField'];
			//Datei wird zum schreiben geoeffnet
			$fp = fopen ($datei_name, "w" );
			//schreibe den Inhalt von Value
			fwrite ( $fp, $_POST['Door_TimeField'] );
			//Datei schliesen
			fclose ( $fp );
			echo "<p><font color='#008000'>Upload successful: " . $dir ."/". $datei_name ."</p>Wert $inhalt in Datei geschrieben</font></p>";
		}
		else{
			echo "<p><font color='#FF0000'>Only numbers are accepted.</font></p>";
		}
	}
}?>
<br>Tor steuern
<form method="get" action="config.php">
<input type="submit" value="Tor zu" name="Torzu">
<input type="submit" value="Tor auf" name="Torauf">
</form>
<!-- PHP code -->
<?php
if (isset($_GET['Torzu'])){
	//exec("sudo i2cset -y 1 0x20 0xFE");//Tor zu
	echo "<font color='#008000'>Tor geht zu!</font>";
}
if (isset($_GET['Torauf'])){
	//exec("sudo i2cset -y 1 0x20 0xFD");//Tor zu
	echo "<font color='#008000'>Tor geht auf!</font>";
}
?>
<hr>
<h2>Telegram</h2>
Satus (0 = inactive | 1 = active)<br>
aktueller Status:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('TelegramStatus.txt');
	echo "$datei_inhalt";
?>
<br><br>
Neuer Modus festlegen:<br>
<!--Save new Value in file-->
<form action="config.php" method="post">

	<input type="radio" name="TelegramStatusField" value="0"> inactive<br>
  <input type="radio" name="TelegramStatusField" value="1" checked="checked"> active<br>

	<input type="submit" name="TelegramStatusInput" value="Speichern"></p>
</form>
<?php
//if the Variable $_POST ['form1'] was defined and the formulr sent the action will be done
if(isset($_POST['TelegramStatusField'])){
	//Dateinamen festlegen
	$datei_name = 'TelegramStatus.txt';
	//Inhalt
	$inhalt = $_POST['TelegramStatusField'];
	//Datei wird zum schreiben geoeffnet
	$fp = fopen ($datei_name, "w" );
	//schreibe den Inhalt von Value
	fwrite ( $fp, $_POST['TelegramStatusField'] );
	//Datei schliesen
	fclose ( $fp );
	echo "<p><font color='#008000'>Upload successful: " . $dir ."/". $datei_name ."</p>Wert $inhalt in Datei geschrieben</p></font></p>";
}
?>
<h2>EMONCMS</h2>
Satus (0 = inactive | 1 = active)<br>
aktueller Status:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('IOTStatus.txt');
	echo "$datei_inhalt";
?>
<br><br>
Neuer Modus festlegen:<br>
<!--Save new Value in file-->
<form action="config.php" method="post">

	<input type="radio" name="IOTStatusField" value="0"> inactive<br>
  <input type="radio" name="IOTStatusField" value="1" checked="checked"> active<br>

	<input type="submit" name="IOTStatusInput" value="Speichern"></p>
</form>
<?php
//if the Variable $_POST ['form1'] was defined and the formulr sent the action will be done
if(isset($_POST['IOTStatusField'])){
	//Dateinamen festlegen
	$datei_name = 'IOTStatus.txt';
	//Inhalt
	$inhalt = $_POST['IOTStatusField'];
	//Datei wird zum schreiben geoeffnet
	$fp = fopen ($datei_name, "w" );
	//schreibe den Inhalt von Value
	fwrite ( $fp, $_POST['IOTStatusField'] );
	//Datei schliesen
	fclose ( $fp );
	echo "<p><font color='#008000'>Upload successful: " . $dir ."/". $datei_name ."</p>Wert $inhalt in Datei geschrieben</p></font></p>";
}
?>
</div>

<!-- ende spalte 1 -->

<!-- spalte 2 -->
<div class="contentbox-1b">
<h3>&Ouml;ffnungszeiten</h3>
24h/7Tage
<p>
Es gibt einige Parameter, die angepasst und veraendert werden koennen.
Hier besteht die Moeglichkeit dies zu tun.
<br>
Man kann den Grenzwert der Helliugkeit anpassen sowie die Hysterese einstellen.
Dies beeinflusst das Hellkigkeitsgesteuerte oeffnen und schliessen des Tores.
<br>
Sollte die Innenraum Temperatur unter einen bestimmten Grenzwert fallen wird ein
Alarm ausgeloesst. Im ersten Ansatz dient dies nur zur Ueberwachung im weiteren Schritt
koennten natuerlich Zusatzfunktionen wie Heizung und dergleichen direkt gesteuert werden.
<br>
<br>
<!--
<h3>Navigation 2nd</h3>
<div class="menu2">
<ul>
<li><a href="index.html" >Link 01</a></li>
<li><a href="index.html" >Link 02 </a></li>
<li><a href="index.html" >Link 03 </a></li>
<li><a href="index.html" >Weitere Links </a></li>
<li><a href="index.html" >More</a></li>
</ul>
 -->

<h3>Betriebsstundenz&auml;hler</h3>
<span><div id="runtime"></div></span>
</div>

<!-- ende spalte 2 -->
</div>
</div>

<!-- ############################## content ENDE ############################## -->
<!-- ############################################################ -->
<!-- fusstop bereich -->
<!-- ############################################################ -->

<div class="fusstop-main">
<div class="fusstop">


<div class="fusstopbox-1a">
<div class="icon-liste">

<ul><!--
<li><i class="fa fa-phone"></i>Phone: (0000) 0000000-50</li>
<li><i class="fa fa-tablet"></i>Mobile : (+49) 0000-1234567</li>
<li><i class="fa fa-fax"></i>Fax: (0000) 7654321-4141</li>
 --></ul>

</div>
</div>

<div class="fusstopbox-1b">
</div>

</div>
</div>

<!-- ############################## fusstop ENDE ############################## -->

<!-- ############################################################ -->
<!-- fuss bereich -->
<!-- ############################################################ -->


<div class="fuss-main">
<div class="fuss">

<div class="fussbox-1a ">
<div class="fussmenu" >
<ul>
<li><a href="../impressum.html">Impressum</a></li>
<li><a href="../datenschutz.html">Datenschutz</a></li>
<li><a href="../disclaimer.html">Disclaimer</a></li>
</ul>
</div>
</div>

<div class="fussbox-1b"><span class="fussname">&copy;2017 | Chickenfarm</span>
</div>
</div>
</div>

<!-- ############################## fuss ENDE ############################## -->

<!-- ============================== ENDE HAUPTBEREICH  ============================== -->

</div>
<!-- ende wrapper-->
</div>
<!-- ende container-haupt-->

</body>
</html>
