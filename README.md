# Comp30023-CS-Project1
>This is the [first project](https://github.com/zsqSeraphina/Comp30023-CS-Project1/blob/master/web_crawler.pdf) of Comp30023 20 s1(13.1/15)

Introduction
============
>- This project aim to build a crawler that fetches urls found from the HTML files got from the url given in stdin.
>- It  should keep sending request to all the fetched urls and search for urls in their HTML files until 100 urls were found.
>- It will print out a log of all the fetched urls.

Implementation
==============
>I splitted this project into 3 parts
### socket.c #
>- The [socket](https://github.com/zsqSeraphina/Comp30023-CS-Project1/blob/master/socket.c) part creates socket connection, sends request from the client side, read the HTML file from the server side, and store it as a string.
### crawler.c #
>- The [crawler](https://github.com/zsqSeraphina/Comp30023-CS-Project1/blob/master/crawler.c) part searches urls in the HTML files get from the server side using regex.
>- Store the urls into a struct called "url_list", which consists of a integer "count" represents the current number of urls in the list and a 2d-array "urls" which stores all the urls satisefies the requirement(specified at part 3.1 of the [instruction](https://github.com/zsqSeraphina/Comp30023-CS-Project1/blob/master/web_crawler.pdf))
### main.c #
>- The [main](https://github.com/zsqSeraphina/Comp30023-CS-Project1/blob/master/main.c) part loops through the "url_list", until all the urls were been fetched and stored into the "url_list".
>- It prints out a log of the urls in the "url_list" and then free the list.

Test
====
>- There is a [test](https://github.com/zsqSeraphina/Comp30023-CS-Project1/blob/master/.gitlab-ci.yml) given by the Tutors, push to run.
>- Or the code itself can be run as ./crawler "url"
