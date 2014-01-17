Ever wondered how you can put resource data in your own database?  Wonder no more!

This can be done from Windows or from Linux:

1)  Run sqlplus and log into your database.
2)  Run the file "src/game/server/database/import/resources/data1.sql".  (To run a file in SQL/Plus, type "@" and the filename.)
3)  Run the file "src/game/server/database/import/resources/data2.sql".
4)  Run "exec resource_importer.import_resources;"
5)  Run "commit;" or exit SQL/Plus.

Voila!

This gives you the data from the most recent version of the spreadsheet that I have imported.  If you want to generate data1 and data2 from the spreadsheet yourself, this is what to do:

1)  Export the resource data page to a tab-delimited text file, data1.txt, in the directory listed above
2)  Export the fractal data page to a tab-delimited text file, data2.txt.
3)  Edit these files to remove the headers and any stray lines.
4)  Run "perl resource_import_delta.txt < data1.txt > data1.sql"
5)  Run "perl resource_fractal_import.txt < data2.txt > data2.sql"
6)  Contiue with step #1 above.
