#expand card project
base pro file, include such function:
1.uart send and recevied,add dma mode with rec
2.spi send and received
3.frame pack struct define 
4.data pack memory malloc and free
5.the data pack decode and encode
6.create send and rec circle buffer,create linklist to manage the buffer info
7.create data pack judge and deal

base pro status:
1.uart send and rec: test and success
2.spi send and rec : no test
3.data pack judege and deal: test uart data and success
4.uart rec with dma: test and success



submit 1:
1.fix EnQueue and DeQueue question, now it is opration the memory.
2.use spi2 as master mode, send data to spi1 as slaver mode, and spi1 success receive data, then deal the rec data correct.
3.the important tip: when use a board as spi master, other board as spi slaver, the interface must be sck--sck,miso--miso,mosi--mosi.
  you can not received correct data pack if use the interface like this:sck--sck,mosi--miso,miso--mosi.

 
submit 2:
1.fix some question about resend to uart data maybe occour error pack data in a frame. the most is the point which point to send string, it is addself after send one pack, then overflow the sendstring limits. now use another point to the send string start addr,and take data use another point self add.
2.use spi2 as master mode, use spi1 as slaver mode, spi1 send a byte data to spi2 success.

submit 3:
1.correctly send spi slaver data to spi master, but send more than one can make error
2.fix spi master reveived data from spi slaver code


submit 4:
1.fix uart reveive queue_buf front and size calculate method, now it is can be used normal
2.use DeQueue method replace memcpy in the uart take data from uart queue to pbuf process. it is can solute the queue save data cycle.
3.use DeQueue method replace memcpy in the spi take data from spi queue to pbuf process,is can be solute queue save data cycle.
4. have a issues about send data to front expand card by use extern uart, when system deal a process about extern uart send data to the master but the process not be end, then send data use the same uart port to the front expand card, the front expand card may be received error length data, if repeat the send operation, sometimes the front expand card received correct data.     
