#!/usr/bin/perl

#
# (c) Copyright 1999-2018 PaperCut Software International Pty Ltd
#
# A simple authentiation provider that users the Samba smbclient command
# to authenticate the user's username and password.  We do this by attempting
# to "list" resources on the specified server with the provided credentials.
#

use strict 'vars';

Main();

sub
Main {

    my $servername = "";

    if ($ARGV[0] =~ /^(\S+)$/) {
        $servername = $1;
    } elsif ($ENV{"SMBSERVER"} =~ /^(\S+)$/) {
        $servername = $1;
    }
    if ($servername eq "") {
        print "ERROR\n";
        die("Error: Samba server not defined in environment var " . 
           " SMBSERVER\n");
    }

    #
    # Test to make sure we have smbclient
    #
    my $smbclient = FindSmbclient();
    if ($smbclient !~ /smbclient/) {
        print "ERROR\n";
        die("Unable to locate smbclient binary.\n");
    }

    #
    # Read username and password from stdin
    #
    my $username = "";
    if (<STDIN> =~ /^(.*)$/) {
        $username = $1;
    }
    my $password = "";
    if (<STDIN> =~ /^(.*)$/) {
        $password = $1;
    }

    if ($password =~ /^\s*$/) {
        print "ERROR\n";
        print STDERR "Blank passwords are denied\n";
        return;
    }

    $ENV{"PASSWD"} = $password;

    my $cmd = "$smbclient -U \"$username\" -L \"$servername\" 2>&1";

    my $output = `$cmd`;
    my $exitstatus = $? >> 8;

    ## Debug output
    ## printf "DEBUG: Command output:\n%s\n", $output;
    ## printf "DEBUG: Exit status: %d\n", $exitstatus;

    if ($exitstatus == 0
            && $output !~ /^\s*$/
            && $output !~ /LOGON_FAILURE/i
            && $output !~ /Anonymous login successful/i) {
        print "OK\n";
    } else {
        print "ERROR\n";
        print STDERR "Invalid username or password\n";
    }
}

sub
FindSmbclient {
    my $path = "";
    my $output = `which smbclient`;
    if ($output =~ /smbclient/) {
        chomp $output;
        $path = $output;
    } else {
        # search in common locations
        my @locations = (
            '/usr/bin/smbclient',
            '/usr/local/bin/smbclient',
            '/usr/local/samba/bin/smbclient',
            '/opt/samba/bin/smbclient',
            '/usr/sbin/smbclient'
            );
        foreach my $filepath (@locations) {
            if (-x $filepath) {
                $path = $filepath;
                last;
            }
        }
    }
    return $path;
}

1;
