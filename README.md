A lambda calculus interpreter from scratch in C because why not

Try equations of the form

(\x.x) y // It applies y to the function \x.x and shows the entire beta reduction

example output

```bash
Lambda> (\x.x) y

Parsed: ((\x.x) y)

Reduction:
 -> y

Normal form: y
```

Church Numerals

```bash
Lambda> (\n.\f.\x.f (n f x)) (\f.\x.f (f x))

Parsed: ((\n.(\f.(\x.(f ((n f) x))))) (\f.(\x.(f (f x)))))

Reduction:
 -> (\f.(\x.(f (((\f.(\x.(f (f x)))) f) x))))
 -> (\f.(\x.(f ((\x.(f (f x))) x))))
 -> (\f.(\x.(f (f (f x)))))

Normal form (Church numeral): 3
```

Church Encoded Booleans

```bash
Lambda> (\x.\y.x)

Parsed: (\x.(\y.x))

Reduction:

Normal form: TRUE

Lambda> (\x.\y.y)

Parsed: (\x.(\y.y))

Reduction:

Normal form: FALSE
```

Try the Y combinator yourslef - (\f.(\x.f (x x)) (\x.f (x x))) g

You can now also run the Typed Lambda calculus interpreter `lamdaty.c`

run `make typed` or `gcc -Wall -Wextra lamdaty.c -o lamdaty -lm` to build it

```bash
STLC> \lambda x : Type . body
```

This can do cool things like

```bash
STLC> (\f:Bool->Bool. \g:Bool->Bool. \x:Bool. f (g x))
        (\b:Bool. b)
        (\b:Bool. b)
        true
Parsed: (\f:(Bool -> Bool).(\g:(Bool -> Bool).(\x:Bool.(f (g x)))))
Type: ((Bool -> Bool) -> ((Bool -> Bool) -> (Bool -> Bool)))

Reduction:

Normal form: (\f:(Bool -> Bool).(\g:(Bool -> Bool).(\x:Bool.(f (g x)))))
Type preserved: ((Bool -> Bool) -> ((Bool -> Bool) -> (Bool -> Bool)))
```

{Type checking and preservation}

We can now easily extend this to a proper funcitonal language
