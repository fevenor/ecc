# basefunc

ECC基本功能

## 功能

ECC基本功能包括随机数产生、生成密钥对、加密及解密。

## 算法

### 随机数产生

随机数使用 Windows API 的 CryptGenRandom 获得，确保获得的随机数为强随机数。

### 加密

```plain
获得公钥PB
将明文编码为一个点M(mx,my)
产生随机数k，计算P=(px,py)=kPB
计算E=(ex,ey)=kG
用P加密明文：C(cx,cy)
cx=mx*px+py
cy=my*py+px
发送(E,C)
```

其中，对于每一组会话，随机数k都不一样，从而避免在多组明文泄露的情况下， px 和 py 被解出。

### 解密

```plain
接收(E,C)
由私钥nB计算P=(px,py)=nB(kG)=k(nBG)=kPB
解密M(mx,my)
mx=(cx-py)/px
my=(cy-px)/py
```