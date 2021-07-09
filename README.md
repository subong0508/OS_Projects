# OS_Projects
2021-1 Operating System Homeworks

## Homework2
### 1. 리눅스 MiniShell 구현
### 2. Merge sort 구현: 기본 구현, 멀티 프로세스 구현, 멀티 스레드 구현 비교

```
cd hw2
make
# MiniShell
./miniShell
# program1, program2, program3
./program1 < {input file}
./program2 {# of process} < {input file}
./program3 {# of thread} < {input file}
# remove executables
make clean
```

## Homework3
### Multilevel queue 스케줄러, 가상 메모리(페이징), 버디 시스템이 결합된 시뮬레이터 구현

```cd hw3
make
./project3 -page={page replacement algorithm} -dir={input directory}
# remove executable
make clean
```
