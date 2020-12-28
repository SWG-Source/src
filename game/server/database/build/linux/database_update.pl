#!/usr/bin/perl
  
# Script to create/initialze/update the database
# Update the database to the latest version by applying delta files
  
use strict;
use Getopt::Long;
  
&main;
  
sub main
{
  
    &GetOptions("delta","drop","create","toversion=s","fromversion=s","packages","noversioncheck","help","username:s","loginusername:s","goldusername:s","password:s","service:s","buildcluster","grantpermissions","grantgoldpermissions","configurelimits","addclusterforlogin","createnewcluster", "newschema", "dbausername:s","dbapassword:s","queries","execquery:s");
  
    if ($::opt_help || !($::opt_drop || $::opt_create || $::opt_delta || $::opt_packages || $::opt_grantpermissions || $::opt_grantgoldpermissions || $::opt_configurelimits || $::opt_addclusterforlogin || $::opt_createnewcluster||$::opt_queries||$::opt_execquery))
    {
    print "Usage: database_update.pl [COMMAND] [OPTION]... \n";
    print "Create a new database or update an existing database to the latest version.\n";
    print "\n";
    print "Commands:\n";
    print "  --create                Create a new database.\n";
    print "  --delta                 Update the database by applying deltas\n";
    print "  --drop                  Drop an existing database.\n";
    print "  --packages              Update the PL/SQL packages. (Done automatically by other options.)\n";
    print "  --createnewcluster      Do all the steps needed to create a new live cluster.\n";
    print "  --grantpermissions      Grant permissions on all the objects in the schema to public\n";
    print "  --grantgoldpermissions  Grant permissions on all the objects in the gold schema to public (useful after importing the gold schema).\n";
    print "  --configurelimits       Prompt for character and account limits stored in the login database,\n";
    print "                            e.g. number of users per cluster\n";
    print "  --addclusterforlogin    Add a cluster to the login database (will prompt for the name)\n";
    print "  --help                  Display these options.\n";
    print "\n";
    print "Options:\n";
    print "  --fromversion=VER       Override checking the database and specify the\n";
    print "                            starting version instead.\n";
    print "  --toversion=VER         Specify the version to which to update the database.\n";
    print "                            (Default:  latest version.)\n";
    print "  --noversioncheck        Don't check the version number after running each update script.\n";
    print "  --username=NAME         Specify the database user name. (Defaults to \$USER.)\n";
    print "  --loginusername=NAME    Specify the login database user name, required for --configurelimits and --addclusterforlogin\n";
    print "  --goldusername=NAME     Specify the gold database user name, required for --grantgoldpermissions\n";
    print "  --password=PASSWORD     Specify the database password.  (Defaults to \"changeme\".)\n";
    print "  --service=NAME          Specify the database service name.  (Defaults to \"swodb\".)\n";
    print "  --buildcluster          Use this option if you set usename = buildcluster\n";
    print "  --newschema             Add this option to auto create schema if it doesn't exist\n";
    print "  --dbausername       Must be used with newschema option\n";
    print "  --dbapassword       Must be used with newschema option\n";
    print "  --queries       Run one time queries from list in automated_queries table\n";
        print "  --execquery             Execute query\n";
    print "\n";
      
    exit;
    }
  
    if ($::opt_createnewcluster)
    {
    # this is equivalent to setting the following options individually
    $::opt_create=1;
    $::opt_grantpermissions=1;
    $::opt_grantgoldpermissions=1;
    $::opt_configurelimits=1;
    $::opt_addclusterforlogin=1;
    }
  
    #check for usernames that must be specified
    if ($::opt_loginusername eq "" && ($::opt_configurelimits || $::opt_addclusterforlogin))
    {
    die "The login database user name must be specified with --loginusername=NAME\n";
    }
    if ($::opt_goldusername eq "" && ($::opt_grantgoldpermissions))
    {
    die "The gold database user name must be specified with --goldusername=NAME\n";
    }
      
    &doDrop if ($::opt_drop);
    &doCreate if ($::opt_create);
    if ($::opt_delta)
    {
    $::didAnUpdate = &doDeltaUpdates($::opt_fromversion,$::opt_toversion,$::opt_noversioncheck);
    }
      
    if ($::didAnUpdate || $::opt_packages)
    {
    &updatePackages;
    }
      
    if ($::opt_grantpermissions)
    {
    &doGrantPermissions;
    }
      
    if ($::opt_grantgoldpermissions)
    {
    &doGrantGoldPermissions;
    }
      
    if ($::opt_configureforlogin)
    {
    &doConfigureForLogin;
    }
  
    if ($::opt_configurelimits)
    {
    &doConfigureLimits;
    }
      
    if ($::opt_addclusterforlogin)
    {
    &doAddClusterForLogin;
    }
  
    if ($::opt_queries)
    {
        &doRunOneTimeQueries;
    }
      
    if ($::opt_execquery)
    {
        &doExecQuery;
    }
  
exit 0;
  
}
  
sub doDrop
{
    system (&sqlplus." @../../queries/drop_all_objects.sql");
}
  
sub doCreate
{
    if ($::opt_toversion)
    {
    die "Cannot specify --toversion with --create\n";
    }
  
    if ($::opt_newschema)
    {
    my($user);
    $user=$::opt_username;
    $user=$ENV{"USER"} if ($user eq "");
  
    my $pwd = $::opt_password;
    $pwd=$ENV{"DB_PASSWORD"} if ($pwd eq "");
    $pwd="changeme" if ($pwd eq "");
  
    open (OUTFILE,"| ".&sqlplusForDBA." > /dev/null");
    print OUTFILE "CREATE USER $user PROFILE DEFAULT IDENTIFIED BY $pwd DEFAULT TABLESPACE DATA TEMPORARY TABLESPACE TEMP ACCOUNT UNLOCK;\n";
    print OUTFILE "GRANT CREATE TYPE TO $user;\n";
    print OUTFILE "GRANT UNLIMITED TABLESPACE TO $user;\n";
    print OUTFILE "GRANT SWG_GENERAL TO $user;\n";
    print OUTFILE "exit;\n";
    close (OUTFILE);
    }
      
    doCreatesInDirectory("../../shared_schema/");
    doCreatesInDirectory("../../schema/");
    doCreatesInDirectory("../../login_schema/");
    doCreatesInDirectory("../../sp_character_schema/");
  
    &updatePackages;
  
    doDataSetupInDirectory("../../shared_data/");
    doDataSetupInDirectory("../../data/");
    doDataSetupInDirectory("../../login_data/");
    doDataSetupInDirectory("../../sp_character_data/");    
}
  
sub doDataSetupInDirectory
{
    my($directory)=@_;
  
    open (CREATELIST,"ls ".$directory."*.sql|");
    while (<CREATELIST>)
    {
    print $_;
    chop;
    system (&sqlplus." < $_ > /tmp/database_update.tmp");
    &checkError("/tmp/database_update.tmp");
    }
    close (CREATELIST);
  
}
  
sub doCreatesInDirectory
{
    my($directory)=@_;
  
    open (CREATELIST,"ls ".$directory."*.type|");
    while (<CREATELIST>)
    {
    print $_;
    chop;
    system (&sqlplus." < $_ > /tmp/database_update.tmp");
    &checkError("/tmp/database_update.tmp");
    }
    close (CREATELIST);
  
    open (CREATELIST,"ls ".$directory."*.tab|");
    while (<CREATELIST>)
    {
    print $_;
    chop;
    system (&sqlplus." < $_ > /tmp/database_update.tmp");
    &checkError("/tmp/database_update.tmp");
    }
    close (CREATELIST);
  
    open (CREATELIST,"ls ".$directory."*.view|");
    while (<CREATELIST>)
    {
    print $_;
    chop;
    system (&sqlplus." < $_ > /tmp/database_update.tmp");
    &checkError("/tmp/database_update.tmp");
    }
    close (CREATELIST);
  
    open (CREATELIST,"ls ".$directory."*.seq|");
    while (<CREATELIST>)
    {
    print $_;
    chop;
    system (&sqlplus." < $_ > /tmp/database_update.tmp");
    &checkError("/tmp/database_update.tmp");
    }
    close (CREATELIST);
  
    open (CREATELIST,"ls ".$directory."*.fkey|");
    while (<CREATELIST>)
    {
    print $_;
    chop;
    system (&sqlplus." < $_ > /tmp/database_update.tmp");
    &checkError("/tmp/database_update.tmp");
    }
    close (CREATELIST);
}
  
# Return the sqlplus command string
sub sqlplus
{
    my ($user,$pwd,$db);
    $user=$::opt_username;
    $pwd=$::opt_password;
    $db=$::opt_service;
    $user=$ENV{"USER"} if ($user eq "");
    $pwd=$ENV{"DB_PASSWORD"} if ($pwd eq "");
    $pwd="changeme" if ($pwd eq "");
    $db="swodb" if ($db eq "");
  
    $user =~ tr/A-Z/a-z/;
    if (($user eq "buildcluster") && !($::opt_buildcluster))
    {
    die "Cannot run on buildcluster unless you use the --buildcluster option\n";
    }
  
    return "sqlplus $user/$pwd\@$db";
}
  
# Return the sqlplus command string with silent option
sub sqlplusSilent
{
    my ($user,$pwd,$db);
    $user=$::opt_username;
    $pwd=$::opt_password;
    $db=$::opt_service;
    $user=$ENV{"USER"} if ($user eq "");
    $pwd=$ENV{"DB_PASSWORD"} if ($pwd eq "");
    $pwd="changeme" if ($pwd eq "");
    $db="swodb" if ($db eq "");
  
    $user =~ tr/A-Z/a-z/;
    if (($user eq "buildcluster") && !($::opt_buildcluster))
    {
        die "Cannot run on buildcluster unless you use the --buildcluster option\n";
    }
  
    return "sqlplus -s $user/$pwd\@$db";
}
  
  
# Return the sqlplus command string for the login schema
sub sqlplusForLogin
{
    my ($user,$pwd,$db);
    $user=$::opt_loginusername;
    $pwd=$::opt_password;
    $db=$::opt_service;
    $pwd=$ENV{"DB_PASSWORD"} if ($pwd eq "");
    $pwd="changeme" if ($pwd eq "");
    $db="swodb" if ($db eq "");
  
    $user =~ tr/A-Z/a-z/;
    if (($user eq "buildcluster") && !($::opt_buildcluster))
    {
    die "Cannot run on buildcluster unless you use the --buildcluster option\n";
    }
  
    if ($user eq "")
    {
    die "Must specify the login user name using --loginusername\n";
    }
  
    return "sqlplus $user/$pwd\@$db";
}
  
# Return the sqlplus command string for the gold schema
sub sqlplusForGold
{
    my ($user,$pwd,$db);
    $user=$::opt_goldusername;
    $pwd=$::opt_password;
    $db=$::opt_service;
    $pwd=$ENV{"DB_PASSWORD"} if ($pwd eq "");
    $pwd="changeme" if ($pwd eq "");
    $db="swodb" if ($db eq "");
  
    $user =~ tr/A-Z/a-z/;
    if (($user eq "buildcluster") && !($::opt_buildcluster))
    {
    die "Cannot run on buildcluster unless you use the --buildcluster option\n";
    }
  
    if ($user eq "")
    {
    die "Must specify the gold user name using --goldusername\n";
    }
  
    return "sqlplus $user/$pwd\@$db";
}
  
# Return the sqlplus command string for dba
sub sqlplusForDBA
{
    my ($user,$pwd,$db);
    $user=$::opt_dbausername;
    $pwd=$::opt_dbapassword;
    $db=$::opt_service;
    $pwd=$ENV{"DB_PASSWORD"} if ($pwd eq "");
    $pwd="changeme" if ($pwd eq "");
    $db="swodb" if ($db eq "");
  
    if (!($::opt_newschema))
    {
        die "Only --newschema may use DBA!\n";
    }
  
    if ($user eq "") 
    {
        die "Must specify the dba user name using --newschema\n";
    }
  
    return "sqlplus $user/$pwd\@$db";
}
  
sub doDeltaUpdates
{
    my($fromversion,$toversion,$noversioncheck)=@_;
    my($updated,$results,$error,$newversion,$version,$invalidlogin,$user,$sqlcommand);
    $updated=0;
    if ($fromversion)
    {
    $version=$fromversion;
    }
    else
    {
    $version=&getVersionNumber;
    }
  
    while ((($toversion ne "") && ($version < $toversion)) ||
       (($toversion eq "") && (-e "../../updates/".($version+1).".sql")))
    {
    $updated = 1;
    print "Updating database to version ".($version+1)."\n";
    $results="";
    open (INFILE,&sqlplus." < ../../updates/".($version+1).".sql|");
    $error=0;
    $invalidlogin=0;
    while ($_=<INFILE>)
    {
        $results.=$_;
        $error=1 if (/ERROR/);
        $invalidlogin=1 if (/invalid username\/password; logon denied/);
    }
    close (INFILE);
      
    if ($error==1)
    {
        print $results."\n";
        if ($invalidlogin = 1)
        {
        $sqlcommand=&sqlplus;
        $user=$1 if ($sqlcommand =~ /sqlplus (\w+)\//);
        #system("echo need to create login $user | /bin/mail -s 'create login $user' aus-db\@soe.sony.com");
        }
        open (OUTFILE,"|".&sqlplus." > /dev/null");
        print OUTFILE "update version_number set version_number=$version;\n";
        print OUTFILE "exit;\n";
        close (OUTFILE);
        die "Could not update database from version $version to version ".($version+1)."\n";
    }
      
    if (!$noversioncheck)
    {
        $newversion=&getVersionNumber();
        if ($newversion != $version+1)
        {
        die "The database reports version number $newversion, but we were expecting version number ".($version+1).".\nThe database may be corrupt, or the update script may be incorrect.\n";
        }
        $version=$newversion;
    }
    }
	
	if($updated == 0)
	{
		print "The database is already up to date at version number ".($version)."\n";
	}
  
    return $updated;
}
  
sub updatePackages
{
    #reload all the packages
      
    updatePackagesInDirectory("../../shared_packages/");
    updatePackagesInDirectory("../../packages/");
    updatePackagesInDirectory("../../login_packages/");
    updatePackagesInDirectory("../../sp_character_packages/");
}
  
sub updatePackagesInDirectory
{
    my($directory)=@_;
    my ($user,$db,$time);
    $time = time();
    $user=$::opt_username;
    $db=$::opt_service;
    $user=$ENV{"USER"} if ($user eq "");
    $db="swodb" if ($db eq "");
  
    #headers
    open (PACKAGELIST,"ls ".$directory."*.plsqlh|");
    while ($_=<PACKAGELIST>)
    {
    chop;
    print "$_\n";
    open (TASK,"|".&sqlplus." > /tmp/$user.packages.$time.tmp");
    print TASK "\@".$_."\n";
    print TASK "exit\n";
    close (TASK);
    &checkError ("/tmp/$user.packages.$time.tmp",1);
    }   
    close(PACKAGELIST);
  
    #package bodies
    open (PACKAGELIST,"ls ".$directory."*.plsql|");
    while ($_=<PACKAGELIST>)
    {
    chop;
    print "$_\n";
    open (TASK,"|".&sqlplus." > /tmp/$user.packages.$time.tmp");
    print TASK "\@".$_."\n";
    print TASK "exit\n";
    close (TASK);
    &checkError ("/tmp/$user.packages.$time.tmp",1);
    }
    close(PACKAGELIST);
}
  
sub getVersionNumber
{
    my ($version);
    $version = -1;
    open (INFILE,&sqlplus." < getversion.sql|");
  
    while ($_=<INFILE>)
    {
    if (/VERSION_NUMBER/)
    {
        die unless ($_=<INFILE>);
        die unless ($_=<INFILE>);
        chop;
        $version = $_ + 0;
    }
    }
    close (INFILE);
  
    die "Could not determine current version number.\n" if ($version == -1);
    return $version;
}
  
sub checkError
{
    my ($filename,$dontdie)=@_;
  
    open (CHECKFILE,$filename);
    while (<CHECKFILE>)
    {
    if (/ERROR/ || /created with compilation errors/ )
    {
        print;
        while (<CHECKFILE>)
        {
        last if (/Disconnected from Oracle/);
        print;
        }
        close CHECKFILE;
        unlink("$filename");
        die unless $dontdie;
        last;
    }
    }
    close (CHECKFILE);
    unlink("$filename");
}
  
sub doGrantPermissions
{
    system (&sqlplus." @../../queries/grant_all_permissions_to_public.sql");
}
  
sub doGrantGoldPermissions
{
    system (&sqlplusForGold." @../../queries/grant_all_permissions_to_public.sql");
}
  
sub doConfigureLimits
{
    print "\nSQL/Plus will prompt for some values:\n";
    print "\tmax_characters_per_account is the number of characters that a single account may have, across all clusters\n";
    print "\tmax_characters_per_cluster is the number of characters that a single cluster may have\n";
    print "\tcharacter_slots is the number of characters that a single account may have on one cluster\n";
    system (&sqlplusForLogin." @../../queries/configure_character_limits.sql");
}
  
sub doAddClusterForLogin
{
   system (&sqlplusForLogin." @../../queries/add_cluster.sql");
}
  
sub doExecQuery
{
   my($execquery, $time, $seq);
  
   #$seq = &getQuerySeq;
   $seq = 0;
   $time = time();
   if ($seq) 
   {
     open (OUTFILE,"| ".&sqlplusSilent." > /tmp/execquery.$::opt_username.$::opt_service.$time.tmp");
     print OUTFILE "insert into automated_queries\n";
     print OUTFILE "(ID, STARTTIME, TYPE, QUERY_TEXT)\n";
     print OUTFILE "VALUES\n";
     print OUTFILE "($seq, util.unix_to_oracle_date($time,-8), 'file','$::opt_execquery');\n";
     print OUTFILE "commit;\n";
     print OUTFILE "exit;\n";
     close (OUTFILE);
     &checkError("/tmp/execquery.$::opt_username.$::opt_service.$time.tmp");
   }
   else 
   {
     print "Couldn't get next sequnce number for query tracking\n";
   }
  
   print " ../../queries/$::opt_execquery\t Started: ",scalar localtime(time()),"\n";
   open (TASK,"|".&sqlplus." > /tmp/execquery.$::opt_username.$::opt_service.$time.tmp");
   print TASK "set serveroutput on\n";
   print TASK "set timing on\n";
   print TASK "\@../../queries/$::opt_execquery\n";
   print TASK "exit;\n";
   close (TASK);
   system("cat /tmp/execquery.$::opt_username.$::opt_service.$time.tmp");
   &checkError("/tmp/execquery.$::opt_username.$::opt_service.$time.tmp");
     
   if ($seq)
   {
     $time = time();
     open (OUTFILE,"| ".&sqlplusSilent." > /tmp/execquery.$::opt_username.$::opt_service.$time.tmp");
     print OUTFILE "update automated_queries\n";
     print OUTFILE "set ENDTIME = util.unix_to_oracle_date($time,-8),\n";
     print OUTFILE "status = 'finished'\n";
     print OUTFILE "where ID = $seq;\n";
     print OUTFILE "commit;\n";
     print OUTFILE "exit;\n";
     close (OUTFILE);
     &checkError("/tmp/execquery.$::opt_username.$::opt_service.$time.tmp");
     print "\n";
     print " ../../queries/$::opt_execquery\t Finished: ",scalar localtime(time()),"\n";
     print "Query text .......\n";
     system("cat ../../queries/$::opt_execquery");
  
   }
   system("rm -f /tmp/execquery.$::opt_username.$::opt_service.$time.tmp");
  
}
  
sub doRunOneTimeQueries
{
   my($row, $rowindex,$colindex, $column, @columns, @data, $time);
  
   $rowindex = 0;
   $colindex = 0;
  
   print scalar localtime(time()),": Checking for one time queries.\n";
  
   open (OUTFILE,"| ".&sqlplusSilent." > /tmp/OneTimeQueries.tmp");
   print OUTFILE "set pagesize 0\n";
   print OUTFILE "set head off\n";
   print OUTFILE "set appi off\n";
   print OUTFILE "set show off\n";
   print OUTFILE "set sqlbl off\n";
   print OUTFILE "set sqln off\n";
   print OUTFILE "set feed off\n";
   print OUTFILE "select rtrim(to_char(id)) || ';' ||rtrim(to_char(util.oracle_to_unix_date(starttime,-8))) || ';' || rtrim(status) || ';' || rtrim(type) \n";
   print OUTFILE "|| ';' || rtrim(to_char(run_serial)) || ';' || rtrim(to_char(exec_order))  || ';' || rtrim(query_text) from automated_queries\n";
   print OUTFILE "where status = 'ready' order by exec_order;\n";
   print OUTFILE "exit;\n";
   close (OUTFILE);
  
  
   open(INFILE,"cat /tmp/OneTimeQueries.tmp |");
   while ($row = <INFILE>)
   {
     @columns = split(/;/,$row);
     foreach $column (@columns)
     {
       $data[$rowindex][$colindex] = $column;
  
       $colindex++;
     }
  
     #print scalar localtime(time()),": Query ID: $data[$rowindex][0] - run as $data[$rowindex][3]\n";
     #print scalar localtime(time()),": Query Text: $data[$rowindex][6]";
     if ($data[$rowindex][2] ne "ready")
     {
           print scalar localtime(time()),": Query $data[$rowindex][0] status is $data[$rowindex][2] ... query will NOT re-execute!\n";
     }
     else
     {
       $time = time();
           #print scalar localtime(time()),": Query $data[$rowindex][0] status is $data[$rowindex][2] starting execute!\n";
           open (OUTFILE,"| ".&sqlplusSilent." > /tmp/database_update.tmp");
       print OUTFILE "update automated_queries set starttime = util.unix_to_oracle_date($time,-8) where ID = $data[$rowindex][0];\n";
           print OUTFILE "commit;\n";
           print OUTFILE "exit;\n";
           close (OUTFILE);
           &checkError("/tmp/database_update.tmp");
  
           #print scalar localtime(time()),": Query $data[$rowindex][0] will be executed serially!\n";
       print " ../../queries/$data[$rowindex][6]";
           open (TASK,"|".&sqlplus." > /tmp/database_update.tmp");
           print TASK "\@../../queries/$data[$rowindex][6]\n";
           print TASK "exit\n";
           close (TASK);
           &checkError("/tmp/database_update.tmp");
             
           $time = time();
           open (OUTFILE,"| ".&sqlplusSilent." > /tmp/database_update.tmp");
           print OUTFILE "update automated_queries set endtime = util.unix_to_oracle_date($time,-8) where ID = $data[$rowindex][0];\n";
           print OUTFILE "commit;\n";
           print OUTFILE "exit;\n";
           close (OUTFILE);
           #print scalar localtime(time()),": Query $data[$rowindex][0] execution complete!\n";
           &checkError("/tmp/database_update.tmp");
             
       open (OUTFILE,"| ".&sqlplusSilent." > /tmp/database_update.tmp");
           print OUTFILE "update automated_queries set status = 'finished' where ID = $data[$rowindex][0];\n";
           print OUTFILE "commit;\n";
           print OUTFILE "exit;\n";
           close (OUTFILE);
           &checkError("/tmp/database_update.tmp");
  
           #print scalar localtime(time()),": Query $data[$rowindex][0] status is finished with no errors!\n";
             
     }
  
     $colindex = 0;
     $rowindex++;
   }
  
   close (INFILE);
  
   print scalar localtime(time()),": There were $rowindex item(s) processed.\n";
  
}
  
sub getQuerySeq
{
   my ($line,$time);
  
   $time = time();
  
   open (OUTFILE,"| ".&sqlplusSilent." > /tmp/seq.$::opt_username.$::opt_service.$time.tmp");
   print OUTFILE "set pagesize 0\n";
   print OUTFILE "set head off\n";
   print OUTFILE "set appi off\n";
   print OUTFILE "set show off\n";
   print OUTFILE "set sqlbl off\n";
   print OUTFILE "set sqln off\n";
   print OUTFILE "set feed off\n";
   print OUTFILE "select rtrim(to_char(seq_queries.nextval)) from dual; \n";
   print OUTFILE "exit;\n";
   close (OUTFILE);
     
   open(INFILE,"cat /tmp/seq.$::opt_username.$::opt_service.$time.tmp|");
   $line = <INFILE>;
   close (INFILE);
   $_=$line;
   if (/seq_queries/)
   {
    return 0;
   }
   system("rm -f /tmp/seq.$::opt_username.$::opt_service.$time.tmp");
   return $line;
  
} 
