# Examples of Custom Authentication and Synchronization modules

<!-- See https://www.papercut.com/kb/Main/CaseStudyCustomUserSyncIntegration
for further information -->

## Introduction

The custom authentication and sync plugin is designed to replace user directory integration
in PaperCut MF/NG for customers who cannot use the existing AD and LDAP integration support.

Once you have created and tested the plugin you will need to install it in PaperCut MF/NG:

In the PaperCut MF/NG admin interface select

```
[options] -> [User/Group Sync]. Under [Primary Sync Source] set these values

[Primary sync source]  => [Custom Program ....]
[Custom user program]  => [<path to your executable plugin>]
[Custom auth program]  => [<path to your executable plugin>]
```

For an overview please refer to the [Custom user directory information providers](https://www.papercut.com/support/resources/manuals/ng-mf/common/topics/customize-user-provider.html) section in PaperCut MF/NG manual.

## Custom user program

Below are the different calls PaperCut will make to a configured custom user program as well as the valid and expected inputs and output.

### Function: is-valid

Test to see if the custom command is valid and appropriately configured for use.

Program Call

```
[CustomUserProgram] - is-valid
```

Example Program Call

```
MyCustomUserProgram.exe - is-valid
```

Standard-input

N/A

Standard-output

`Y\y` when custom provider is functioning and valid.
Or; `N\n` when is not functioning and valid.

Trigger(s)

This is called on application server start, or setting page access and is used to validate that the custom provider is functional and working as expected. An example maybe to say check that the directory source is reachable. Most custom implementations will simply return Y\n.

### Function: all-users

List all users on the network and their information (e.g. email address, office, department, …).

Program Call

```
[CustomUserProgram] - all-users
```

Example Program Call

```
MyCustomUserProgram.exe - all-users
```

Standard-input

N/A

Standard-output

List of user details (see below for more information)

Standard-error

Only on error. Return an error message [error message string/description]\n. An error may be an event such as the inability to contact the directory server.

Exit Code

Zero on success and non-zero on error (combine with error message on STDERR).

Trigger(s)

The command is called when PaperCut needs to list all system users. For example during a full network wide group sync. It many be called manually using Synchronize Now under the Options → User/Group Sync tab, or overnight as part of the automated sync.

### Function: all-groups

List all user directory groups available.

Program Call

```
[CustomUserProgram] - all-groups
```

Example Program Call

```
MyCustomUserProgram.exe - all-groups
```

Standard-input

N/A

Standard-output

`groupName\n` for each group. The output must be UTF-8 encoded.

Standard-error

Only on error return on standard error an error message [error message string/description]\n.

Exit Code

Zero on success and non-zero on error (combine with error message on STDERR).

Trigger(s)

This command is called when PaperCut needs to list all groups on the network. Primarily it’s called when the system Administrator attempts to add groups to the Groups section so they can use this group for reporting or rules.

### Function: get-user-details

List details about an individual user (e.g. email address, office, department, …).

Program Call

```
[CustomUserProgram] - get-user-details
```

Example Program Call

```
MyCustomUserProgram.exe - get-user-details
```

Standard-input

```
Single username\n
```

Standard-output

User details (see below for more information) for the specific user, UTF-8 encoded and terminated with a newline.

Standard-error

Only on error return on standard error an error message [error message string/description]\n.

Exit Code

Zero on success and non-zero on error (combine with error message on STDERR).

Trigger(s)

This command is called to fetch information about a single use. It’s called when a user is created in an individual operation (e.g. on first print) rather than a batch operation (e.g. a full user/group sync).

### Function: group-member-names

List all members of a group. This command produces a list of usernames only. No user information is provided.

Program Call

```
[CustomUserProgram] - group-member-names [groupName]
```

Example Program Call

```
MyCustomUserProgram.exe - group-member-names “Engineering”
```

Standard-input

N/A

Standard-output

List of usernames, each one separated by a newline UTF-8 encoded

Standard-error

Only on error return on standard error an error message [error message string/description]\n. An error may include a event such as a call for a non-existent group.

Exit Code

Zero on success and non-zero on error (combine with error message on STDERR).

Trigger(s)

This command is called to list all members in a group. It’s called as part of a full user/group sync.

### Function: group-members

This command is similar to Group-Member-Names, however it returns user information as well as the username.

Program Call

```
[CustomUserProgram] - group-members [groupName]
```

Example Program Call

```
MyCustomUserProgram.exe - group-members “Engineering”
```

Standard-input

N/A

Standard-output

List of user details (see below for more information)

Standard-error

Only on error return on standard error an error message [error message string/description]\n. An error may include a event such as a call for a non-existent group.

Exit Code

Zero on success and non-zero on error (combine with error message on STDERR).

Trigger(s)

This command is called if the administrator has selected a group as a sync source.

### Function: is-user-in-group

Test to see if a user is in a given group.

Program Call

```
[CustomUserProgram] - is-user-in-group [groupName] [username]
```

Example Program Call

```
MyCustomUserProgram.exe - is-user-in-group “Engineering” “bobsmith”
```

Standard-input

N/A

Standard-output

`Y` or `y` when user is  present in group

Or; `N` or `n` when user is not present in group

Standard-error

Only on error return on standard error an error message `error message string\n`. An error may include a event such as a call for a non-existent user.

Exit Code

Zero on success (user present in group or not present in group) and non-zero on error (combine with error message on STDERR).

Trigger(s)

This command is called when a user is added to the system in an individual even such as a on-first-print event. The information is used to see if group based setup rules should apply to this user.

## List of user details

The custom user program must provide a list of user details in response to the `all-users`, `get-user-details`, and `group-members` requests.

The user details is an ordered list of fields each separated by a tab character (`\t`) and terminated with a newline. The text should be UTF-8 encoded.

Each field is optional and may be supplied as a zero length string `\t\t`. Only the username is a mandatory.

The complete list of fields is

* User Name
* Full Name
* Email
* Department
* Office
* Primary Card Number
* Other Emails
* Secondary Card Number
* Username Alias (secondary username)
* Home Directory
* PIN

For example

```
janer\tJane Rodgers\tjaner@here.com\tSales\tDocklands\n
```

```
jane\tJane Rodgers\tjaner@here.com\tSales\tDocklands\t5678\tpersonal2@webmail.com\t05678\tuser2\t\\server\dfs\homedirs\user2\t1234\n
```

or

```
jane\tJane Rodgers\tjaner@here.com\tSales\tDocklands\t5678\tpersonal2@webmail.com\t05678\t\t\t1234\n
```

NOTE: The fields must be supplied in the **correct order**.

## Custom authentication program

Here are the details PaperCut MF/NG will make to a configured Custom auth program as well as the valid and expected inputs and output. A custom authentication program is used to validate a user’s username and password.

Program Call

```
[CustomAuthProgram]
```

Example Program Call

```
MyCustomAuthProgram.exe
```

Standard-input

`username\nplaintextPassword\n`. The input must be UTF-8 encoded.

Standard-output

`OK\n` when authentication is successful.

Or `OK\nusername\n` when authentication is successful. This is useful for removing diacritic marks from usernames. PaperCut will use the username returned instead.

Or; `ERROR\n` if the username and password is invalid, or a general error of any description occurs.

Standard-error

Only on error or invalid authentication return on standard error an error message [error message string/description]\n. In line with best security practice on authentication failure return “Invalid username or password\n”. Do not disclose any additional information such as the existence of a valid username or invalid password via independent messages.

Exit Code

The exit code should always be zero (0).

Trigger(s)

This program is called only when authentication of a user with password is required. It is not called if
the user uses swipe card or uses an ID to login.


## Synchronisation Workflow

The following is the order each step occurs in during a typical manual or overnight use synchronization run.

1. `all-users`, or `group-members` if syncing against a group
2. `group-member-names` is then called for each active group listed in PaperCut MF/NG
