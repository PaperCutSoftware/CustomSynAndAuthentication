# How to Create & Manage Users in PaperCut MF

**Note this information also applies to PaperCut NG**

There are a number of ways that user accounts get created in PaperCut
MF/NG and this non technical description tries to clear up any
confusion. Because the article is introductory and non technical, we are
going to gloss over a lot of the details.

In PaperCut you can have two types of users: External and Internal. By
default internal users are disabled, so let's talk about them later.

## External Users

Most organisations have a database somewhere that keeps a list of all
the computer users. It's called the user directory and technical people
often call it the AD server or LDAP server.

The user directory contains a wide variety of information about users
(it's often slightly different between organisations) such as the user's
full name, dept, email address, card numbers and so on. Most importantly
the user directory will know the username and password plus any group
memberships the user has.

Group and group membership allows PaperCut to manage users in various
ways. For example, making sure that everyone in the accounts department
gets their printing charged to the correct budget, or that students
can't do colour photocopying.

The user directory service might be running in your IT centre, or it
might be running on a cloud service: for example Google Cloud; or
Microsoft Azure.

In order to track print jobs PaperCut needs to know the username (all
print jobs have the username attached) and, in order to allow users to
access copying or scanning by swiping an access card, PaperCut MF/NG
also needs to know about the user's card numbers.

Every night PaperCut MF/NG reads the user accounts from the user
directory and adds new user accounts (it also updates any details that
have changed and can remove any users that have been deleted). It's very
important to know that during this process PaperCut MF/NG does NOT store
the user password, and in fact NEVER knows the password for external
user accounts.

So from this information a couple of questions come up.

1.  How does an external user login in if PaperCut MF/NG does not know
    the password?

    1.  If the user gives their username and password to PaperCut it is
        passed to the user directory service, in real time, for
        validation.

2.  How does a new user use PaperCut MF/NG on their 1st morning when the
    user account was not in the user directory the night before during
    the synchronisation process?

    1.  Usually the user account is created "on demand" and PaperCut
        will immediately ask the user directory service for account
        information about this one user. Other options are possible
        based on the needs of the organisation.

By default PaperCut MF/NG will add all the users in the user directory
to its database. However, if needed you can restrict PaperCut MF/NG to
only add users from a few groups.

Some organisations may have a custom user directory that does not
support AD or LDAP. In this case a small interface module can be created
to connect to PaperCut MF/NG. More information at
https://github.com/PaperCutSoftware/CustomSynAndAuthentication.

## Internal Users

So for most organisations connecting PaperCut MF/NG to their central
user directory gives them everything they want. However sometimes you
need [internal users](https://www.papercut.com/support/resources/manuals/ng-mf/applicationserver/topics/user-guest-internal.html), mainly when

1.  You have no central database of users, except maybe a spreadsheet or 
    some user management tool that does not support LDAP, or

2.  As well as your normal users, who's details you store in the user
    directory service, you have other users (for instance temporary
    contractors) who need access to PaperCut MF/NG.

For these users you can enable the "internal users" feature and add them
directly to the PaperCut MF/NG database. The easy way to remember the
difference between internal and external users is that for internal
users PaperCut MF/NG stores and manages the user's password. External
user's do not have their passwords stored in PaperCut MF/NG.

Note that in terms of user functionality there is no difference between
internal and external users. It's only the management of the user
account details and group membership that is different.

You can create internal user accounts in a variety of ways:

-   Use the PaperCut MF/NG admin interface via the web browser
  
-   Import user accounts in a text file

-   Use an [API integration](https://www.papercut.com/kb/Main/IDMIntegration)
    with another system to maintain account details
    (this is a specialist approach and you should talk to your PaperCut
    technical account manager first)

-   With the correct configuration users can walk up to a web browser
    and create their own account (explained in the PaperCut MF manual).

## In Summary:

1.  Most customers only need to use external user accounts by connecting
    PaperCut MF/NG to the organisation's user directory service
2.  PaperCut MF/NG never knows external users' passwords. It asks the
    directory service to validate user names and passwords
3.  PaperCut MF/NG uses group membership to manage and control a number
    of features and this membership information is obtained from the
    directory service
4.  PapeCut MF/NG does store card or ID numbers -- it gets these values
    from the directory service, or somewhere else
5.  PaperCut MF/NG loads user account information from the directory
    service overnight
6.  PaperCut can synchronise all the user accounts, or only users from
    selected groups
7.  If a user tries to use PaperCut MF/NG and is not in the MF/NG
    database the directory service is asked for information about that
    one user
8.  Internal user accounts are stored in the PaperCut MF/NG database and
    include the user's password
9.  Internal and external user accounts have identical functionality
10. It's also possible to manage groups and group membership in PaperCut
    MF/NG but that is beyond the scope of this document
11. All of these features are controlled and managed by the PaperCut
    MF/NG Admin
