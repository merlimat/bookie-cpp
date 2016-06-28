# Bookie-cpp

Proof of concept C++ server implementing Apache BookKeeper server functionality

### Dependencies
 * C++ 14 compiler
 * [Folly](https://github.com/facebook/folly/)
 * [Wangle](https://github.com/facebook/wangle/)
 * [ZooKeeper C client](https://zookeeper.apache.org/)
 * [Log4cxx](https://logging.apache.org/log4cxx/)
 
### Compile 
```shell
cmake .
make 
```

### Run the bookie

```
./bookie -h
Allowed options:
  -h [ --help ]                                    This help message
  -z [ --zkServers ] arg (=localhost:2181)         List of ZooKeeper servers
  --zkSessionTimeout arg (=30000)                  ZooKeeper session timeout
  --bookieHost arg (=localhost)                    Boookie hostname
  -p [ --bookiePort ] arg (=3181)                  Bookie TCP port
  -d [ --dataDir ] arg (=./data)                   Location where to store data
  -w [ --walDir ] arg (=./wal)                     Location where to put RocksDB Write-ahead-log
  -s [ --fsyncWal ] arg (=1)                       Fsync the WAL before acking the entry
  -r [ --statsReportingIntervalSeconds ] arg (=60) Interval for stats reporting
```

Test client 

```
./perfClient -h
  -h [ --help ]                         This help message
  -a [ --bookieAddress ] arg (=localhost:3181)
                                        Boookie hostname and port
  -r [ --rate ] arg (=100)              Add entry rate
  -s [ --msg-size ] arg (=1024)         Message size
  -c [ --num-connections ] arg (=16)    Number of connections
  --format-stats arg (=1)               Format stats JSON output
  --stats-reporting arg (=10)           Interval to report latency stats in
                                        seconds
```                                        
