#!/usr/bin/perl

#
# (c) Copyright 1999-2018 PaperCut Software International Pty Ltd
#
# A user directory information provider implementation based on Samba.
# This is simply a Perl script that wraps the Samba "net" command.
#

use strict 'vars';

#
# Global variables
#
my $Server;
my $Username;
my $Password;
my $NetCmd;

Main();


sub
Main {

    if (@ARGV < 2) {
        die("Error: Invalid Command\n" . 
        "Usage: options-string command [arguments...]\n" .
        "  options-string - unused (can pass in dummy argument)\n" .
        "  command:\n" .
        "    is-valid - does net command actually work\n" .
        "    all-users\n" .
        "    all-groups\n" .
        "    group-member-names group\n" .
        "    group-members group\n" .
        "    is-user-in-group user group\n" .
        "    get-user-details\n"
        );
    }

    $NetCmd = FindNetCommand();

    if ($ARGV[1] eq "is-valid") {
        IsValid();
        exit;
    }

    if ($NetCmd =~ /^\s*$/) {
        die("Error: Unable to locate Samba 'net' command\n");
    }

    #
    # Extract the username and password from environment variables
    #
    if ($ENV{"SMBUSER"} =~ /^\s*$/) {
        die("Error: Samba username not defined in SMBUSER\n");
    }
    if ($ENV{"SMBPASSWD"} =~ /^\s*$/) {
        die("Error: Samba password not defined in SMBPASSWD\n");
    }
    if ($ENV{"SMBSERVER"} =~ /^\s*$/) {
        die("Error: Samba server not defined in SMBSERVER\n");
    }

    $Username   = $ENV{"SMBUSER"};
    $Password   = $ENV{"SMBPASSWD"};
    $Server     = $ENV{"SMBSERVER"};

    if ($ARGV[1] eq "all-users") {

        AllUsers();

    } elsif ($ARGV[1] eq "all-groups") {

        AllGroups();

    } elsif ($ARGV[1] eq "group-member-names") {

        if ($ARGV[2] =~ /^\s*$/) {
            die ("Error: group-member-names needs an argument\n");
        }
        GroupMemberNames($ARGV[2]);

    } elsif ($ARGV[1] eq "group-members") {

        if ($ARGV[2] =~ /^\s*$/) {
            die ("Error: group-members needs an argument\n");
        }
        GroupMembers($ARGV[2]);

    } elsif ($ARGV[1] eq "is-user-in-group") {

        if ($ARGV[2] =~ /^\s*$/ || $ARGV[3] =~ /^\s*$/) {
            die ("Error: Missing arguments for is-user-in-group\n");
        }
        IsUserInGroup($ARGV[2], $ARGV[3]);

    } elsif ($ARGV[1] eq "get-user-details") {
        # Do nothing, we don't support user details

    } else {

       die("Error: Unknown Command\n");

    }
}


sub
IsValid {

    my $out;

    # Check that Samba net command exists.
    if ($NetCmd =~ /^\s*$/) {
        OutputBoolean(0);
    }

    # Check we can read the version (ie. command outputs something).
    $out = `$NetCmd --version`;
    if ($out =~ /^\s*$/) {
        OutputBoolean(0);
    }

    OutputBoolean(1);
}

sub
FindNetCommand {
    my $path = "";
    my $output = `which net`;
    if ($output =~ /net/) {
        chomp $output;
        $path = $output;
    } else {
        # search in common locations
        my @locations = (
            '/usr/bin/net',
            '/usr/local/bin/net',
            '/usr/local/samba/bin/net',
            '/opt/samba/bin/net',
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

sub
AllUsers {

    $ENV{"PASSWD"} = $Password;
    # Previously the "rpc" arg was placed before "user" (after the options).
    # This was changed after reportedly not working on Samba 3.4.8.
    my $cmd = "$NetCmd rpc -S \"$Server\" -U \"$Username\" user 2>/dev/null";

    open NET, "$cmd |" || die("Unable to exec samba net command\n");
    my $username;
    while(<NET>) {
        if (/^(.*)$/) {
            $username = $1;
            if (IsValidUser($username)) {
                print "$username\t\t\n";
            }
        }
    }
}


sub
AllGroups {

    $ENV{"PASSWD"} = $Password;
    my $cmd = "$NetCmd -S \"$Server\" -U \"$Username\" rpc group 2>/dev/null";

    open NET, "$cmd |" || die("Unable to exec samba net command\n");
    my $groupname;
    while(<NET>) {
        if (/^(.*)$/) {
            $groupname = $1;
            if (IsValidGroup($groupname)) {
                print "$groupname\n";
            }
        }
    }
}


sub
GroupMemberNames {

    my ($groupname) = @_;

    my @members = GetGroupMembers($groupname);
    my $member;
    foreach $member (@members) {
        print "$member\n";
    }
}


sub
GroupMembers {

    my ($groupname) = @_;

    my @members = GetGroupMembers($groupname);
    my $member;
    foreach $member (@members) {
        # We don't have any fullname or email address with this provider
        print "$member\t\t\n";
    }
}


sub
IsUserInGroup {
    my ($username, $groupname) = @_;

    my @members = GetGroupMembers($groupname);

    my $member;
    foreach $member (@members) {
        if ($username eq $member) {
            # user found
            OutputBoolean(1);
            return;
        }
    }
    # user not found
    OutputBoolean(0);
}


sub
GetGroupMembers {

    my ($groupname) = @_;

    $ENV{"PASSWD"} = $Password;
    my $cmd = "$NetCmd -S \"$Server\" -U \"$Username\" rpc group members " .
	 "\"$groupname\" 2>/dev/null";

    my $NET;
    open $NET, "$cmd |" || die("Unable to exec samba net command\n");

    my $username;

    # We only care about the key in %members, not the value.
    # Use associative array to eliminate duplicate users from multiple nested groups.
    my %members;

    while(<$NET>) {
        if (/^(.*)$/) {
            $username = $1;
            # String domain suffix if exists
            if ($username =~ /.+\\(.*)$/) {
                $username = $1;
            }
            if (IsValidUser($username)) {
                $members{$username}++;
            }
            if (IsGroup($username)) {
                # The username is actually really a group name
                # So we now need to find out all its users using
                # a recursive call to this routine
                delete $members{$username}; # delete as not really a user
                my @nested_members = GetGroupMembers($username);
                my $nested_member;
                # Add all nested members to our associative array
                foreach $nested_member (@nested_members) {
                    $members{$nested_member}++;
                }
            }
        }
    }
    # sort the output, otherwise it gets quite disorganized with multiple nested groups
    return sort keys %members;
}


sub
IsValidUser {
    my ($username) = @_;

    if ($username =~ /\$$/) {
        return 0;
    } elsif ($username =~ /^iusr_/i) {
        return 0;
    } elsif ($username =~ /^iwam_/i) {
        return 0;
    } elsif ($username =~ /^support_/i) {
        return 0;
    } elsif ($username =~ /^helpassistant/i) {
        return 0;
    } elsif ($username =~ /^aspnet/i) {
        return 0;
    } elsif ($username =~ /^krbtgt/i) {
        return 0;
    } elsif ($username =~ /^__vmware_user__/i) {
        return 0;
    }
    return 1;
}


sub
IsValidGroup {
    my ($groupname) = @_;

    if ($groupname =~ /\$$/) {
        return 0;
    } elsif ($groupname =~ /^none$/i) {
        return 0;
    } elsif ($groupname =~ /^__vmware__$/i) {
        return 0;
    }
    return 1;
}

sub
IsGroup {
    my ($groupname) = @_;

    $ENV{"PASSWD"} = $Password;

    # Only check the return status.  If the group exists (and no other errors) it's ZERO.
    `$NetCmd rpc -S \"$Server\" -U \"$Username\" group members \"$groupname\" 2>/dev/null`;
    if ($? == 0) {
        return 1;
    }
    return 0;
}



sub
OutputBoolean {
    my ($b) = @_;
    if ($b) {
        print "Y\n";
    } else {
        print "N\n";
    }
}

1;
