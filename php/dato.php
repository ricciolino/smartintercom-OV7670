
<?php
$filename = 'img.txt';
$dato = $_POST['dato'];
$len = strlen($dato);

set_time_limit(5*60);
// Let's make sure the file exists and is writable first.
if (is_writable($filename)) {

    // In our example we're opening $filename in append mode.
    // The file pointer is at the bottom of the file hence
    // that's where $dato will go when we fwrite() it.
    if (!$handle = fopen($filename, 'a')) {
         echo "Cannot open file ($filename)";
         exit;
    }

    // Write $dato to our opened file.
    if (fwrite($handle, $dato) === FALSE) {
        echo "Cannot write to file ($filename)";
        exit;
    }

    echo "$len";

    fclose($handle);

} else {
    echo "The file $filename is not writable";
}
?>

