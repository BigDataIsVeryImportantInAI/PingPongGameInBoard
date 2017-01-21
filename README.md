# PingPong Game

----
## 개발 배경/목표

>* 임베디드 프로그램을 작성하기 위한 보드 구동 환경, 프로그래밍 작성을 위한 컴파일러 구현, LCD에 출력하기 위한 출력관련 프로그램을 다 숙달한 뒤 기존 커리큘럼에서 배운 통신을 주제로 게임을 만듬

>* 보드가 서버가 되어 게임을 구동시키고 다른 리눅스 환경의 PC로 접속을 하면 핑퐁 게임을 즐길 수 있다.


----
## 개발 구성 및 과정

>* 리눅스 환경에서 Client 실행파일로 Board의 서버에 접속할 수 있다. 

>* 클라이언트는 처음에 Insert Coin화면 창에서 1 혹은 2 키를 입력하여 하나의 플레이어 번호로 접속 할 수 있고 두 명의 플레이어가 각자 1, 2키를 입력해 모두 접속이 되면 2초후 게임이 시작된다. 그 후 플레이어들은 각자의 Bar를 w키와 s키로 위, 아래 조정을 하여 날라오는 공을 막고 상대방 쪽으로 공을 날려 점수를 획득한다. 상단에 점수가 표기된다.

>* 서버는 클라이언트로부터 받은 접속 여부로 게임을 시작할 수 있고 w, s값으로 Bar의 좌표를 변경, 출력해준다.

>![개발 구성](/image/concept.png)

----
## 개발 내용

>* 크로스 컴파일러를 설치하여 보드에 프로그램을 올릴 수 있게 한다.

>* 서버는 Thread를 두 개 사용하여 하나는 UDP Socket 생성 후 recvform()으로 데이터를 받는 역할만 하고, 다른 하나는 얻은 데이터로 출력만 해주는 기능을 한다. 옆의 그림은 이와 같은 구조를 Block Diagram으로 표시한 것이다.

>![블럭 다이어그램](/image/diag1.png)

>![블럭 다이어그램](/image/diag2.png)

>* 클라이언트는 하나의 실행 흐름으로 UDP Socket 생성 후 sendto()로 데이터를 전송하는 역할만 한다. 

>![블럭 다이어그램](/image/diag3.png)

>* 네모를 그리는 함수, 원을 그리는 함수를 작성하여 핑퐁 게임에 맞게 배치한다.

>* 소켓 통신으로 받는 2개의 컴퓨터에서 오는 데이터를 구분하여 각각의 Bar를 움직인다.


----
## 개발 관련 사항

>* 개발 언어 : C

>* PC : Linux Fedora

>* Board : Marsboard RK3066 Feature

>* Protocol : UDP Socket

>* 담당분야 : 전체

----
## 실행 화면

>![실행화면](/image/res1.png)

>![실행화면](/image/res2.png)

----
## 패키지 설명

>* 소스 : socket/client.c, ttt_test/main.c

>* 실행파일 : client, server

>* 보드 nfs 추가파일 : fond/, lib/