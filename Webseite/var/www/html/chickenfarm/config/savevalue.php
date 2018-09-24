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
<!-- Der nachfolgende einzeilige Copyright-Vermerk (c) ist nicht zu löschen.-->
<!-- (c)Copyright by S.I.S.Papenburg / www.on-mouseover.de/templates/ -->
<!--Ein Entfernen dieses Copyright/Urheberrecht-Vermerks kann rechtliche Schritte nach sich ziehen -->
<!-- ////////////////////////////////////// -->
<link href="../images/chicken.ico" rel="shortcut Icon"/>
<link rel="stylesheet" href="../css/menue.css" type="text/css">
<link rel="stylesheet" href="../css/format.css" type="text/css">
<link rel="stylesheet" href="../css/animation.css" type="text/css">
<link rel="stylesheet" href="../css/font-awesome.min.css" type="text/css">
<link rel="stylesheet" href="../css/font-roboto-light.css" type="text/css">
<link rel="stylesheet" href="../css/font-open-sans-light.css" type="text/css">


<!-- /////////////////// BROWSERWEICHE  - Conditional Comments für Internet Explorer Vers. 8 und tiefer /////////////////// -->
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
	$('#runtime').load('../../tmp/runtime.txt');
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
<!-- Menü -->
<div id="menu1">
<ul>
<li><a  href="../index.html">Home</a></li>
<li><a  href="../bewohner.html">Bewohner</a></li>
<li><a  href="../webcam.html">Webcam</a></li>
<li id="aktuell"><a  href="config.html">Konfiguration</a></li>
<li><a  href="../../pic/index.php">Pi Control</a></li>
<!-- wird aktuell nicht benötigt
<li><a  href="index4.html">Quartus</a></li>
<li><a  href="index5.html">Quintus</a></li>
<li><a  href="index6.html">Sextus</a></li>
<li><a  href="index7.html">Septus</a></li>
 -->
</ul>
</div>
<!-- Menü ende  -->
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

<h1>Konfiguration</h1>
<h2>Helligkeit</h2>
Der aktuelle Grenzwert betraegt:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('../../config/threshold.txt');
	echo $datei_inhalt;
	$dir = dirname(__FILE__);
	echo "<p>Speicherpfad: $dir </p>";
?>

<!--Save new Value in file
<form action="savevalue.php" method="post">
<p>Neuer Wert:<input type="Text" name="Value"></p>	
<input type="submit" name="" value="save">
</form>
-->	
<?php
if (empty ($_POST['Value']) == TRUE ){
	echo "<p><font color='#FF0000'>Upload failed. Empty Cell</font></p>";
	//Seite neu laden
	header("Refresh:0");
}
	else{
		//Abfrage ob nur Ziffern verwendet werden
		if (ctype_digit($_GET['Value'])){
			//Dateinamen festlegen
			$datei_name = '../test.txt';
			//Datei wird zum schreiben geoeffnet
			$fp = fopen ($datei_name, "w" );
			//schreibe den Inhalt von Value
			fwrite ( $fp, $_GET['Value'] );
			//Datei schliesen
			fclose ( $fp );
			echo "<p><font color='#008000'>Upload successful: " . $dir ."/". $datei_name ."</font></p>";

			//Datei nicht weiter ausfuehren
			exit;}
		else{
			echo "<p><font color='#FF0000'>Only numbers are accepted.</font></p>";
		}
	}
?>
Die aktuelle Hysterese beträgt:
<?php
	$datei_inhalt = file_get_contents('../../config/hysteresis.txt');
	echo "<p>$datei_inhalt</p>";
	$path_parts = pathinfo('../test.txt');

	echo $path_parts['dirname'], "\n";
	echo $path_parts['basename'], "\n";
	echo $path_parts['extension'], "\n";
	echo $path_parts['filename'], "\n"; // seit PHP 5.2.0


?>

</div>
<!-- ende spalte 1 -->

<!-- spalte 2 -->
<div class="contentbox-1b">
<h3>&Ouml;ffnungszeiten</h3>
24h/7Tage
<p>
Hier erfahren sie einige Einzelheiten sowie Besonderheiten der Bewohner.
<br>
<br>
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

<h3>Betriebsstundenzähler</h3>
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