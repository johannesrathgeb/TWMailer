TODO: 

-LIST command
-READ command
-DEL command
-QUIT command (eh eigl fertig, nur noch einfügen)



client.cpp: 

-input/error handling der inputs für die email (Z44 - Z63)


server.cpp: 
-Problem: ich glaub du hast einen buffer in einer Funktion und global erstellt, der gleich benannt ist; 
hat beim erstellen von mehr als einer email beim übergeben vom client zum server für Fehler gesorgt, 
deswegen hab ich den globalen Buffer und die receiveMessage mal auskommentiert. das müsst noch gefixed werden. 

beides/anderes:

-hab die Client/Server Communication mit OK/ERR usw. jz nicht mehr kontrolliert, gut möglich, dass ich das was vergessen hab,
müsst ma uns morgen noch anschauen. 

-makefile

-vll das mit der Porteingabe am Anfang noch ändern, glaub das sollt nicht auf default laufen aber ka
-<mail-spool-directoryname> noch hinzufügen
