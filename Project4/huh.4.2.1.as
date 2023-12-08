	lw	0	1	five
	lw	0	4	SubAdr
start	jalr	4	7
Goob	lw	0	3	Nope
Nope	beq	0	1	done
	beq	0	0	start
done	halt
five	.fill	5
six		.fill	John
