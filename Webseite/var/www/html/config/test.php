<html>
<head>
<Titel> GPIO über HTML schalten</Titel>
</head>
<body>
GPIO schalten
<form method="get" action="test.php">
<input type="submit" value="Tor zu" name="Torzu">
<input type="submit" value="Tor auf" name="Torauf">
</form>

<!-- PHP code -->
<?php
if (isset($_GET['Torzu'])){
	exec("sudo i2cset -y 1 0x20 0xBF");//Tor zu
	echo "Tür geht zu!";
}
if (isset($_GET['Torauf'])){
	exec("sudo i2cset -y 1 0x20 0x7F");//Tor zu
	echo "Tür geht auf!";
}
?>






Der aktuelle Grenzwert betr&auml;gt:
<!--Read out File for cvs filename-->
<?php
	$datei_inhalt = file_get_contents('test.txt');
	echo "$datei_inhalt LUX";
?>
<!--Save new Value in file-->
<form action="test.php" method="post">
<p>Neuer Wert:<input type="Text" name="LUXthresholdInputField">
<input type="submit" name="LUXthresholdInput" value="Speichern"></p>
</form>

<?php
// Schreiben des neuen Wertes
$inhalt = $_POST['LUXthresholdInputField'];
$handle = fopen ("test.txt", w);
fwrite ($handle, $inhalt);
fclose ($handle);
 
echo "Wert $inhalt in Datei test.txt  geschrieben";
?>

	
</body>
</html>



