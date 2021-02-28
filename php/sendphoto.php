<?php

#create txt file
system("cat head160x120.bmp > final160x120.bmp", $res);
system("cat img.txt >> final160x120.bmp", $res);

#sudo apt install php php-curl
$chat_id="<chat_id>";
$bot_url = "https://api.telegram.org/bot<bot_number>:<bot_string>/";
$url = $bot_url . "sendPhoto?chat_id=" . $chat_id;
$post_fields = array( 'chat_id'   => $chat_id, 'photo'     => new CURLFile(realpath("final160x120.bmp") ) );

$ch = curl_init(); 
curl_setopt($ch, CURLOPT_HTTPHEADER, array( "Content-Type:multipart/form-data" ) );	
curl_setopt($ch, CURLOPT_URL, $url); 
curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
curl_setopt($ch, CURLOPT_POSTFIELDS, $post_fields); 
$output = curl_exec($ch);

?>
