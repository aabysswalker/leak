#[derive(Debug)]
pub enum Errors {
    CannotCreateString,
}

#[derive(PartialEq, Eq, Debug, Clone)]
pub enum TokenType {
    SemiColon,
    DoubleQuoteStart,
    DoubleQuoteEnd,
    Ident,
    StringLiteral,
    Number,
    Comma,
    AssignOp,
    SqBracketOpen,
    SqBracketClose,
    CuBracketOpen,
    CuBracketClose,
    ForStatement,
    IfKeyword,
    ElseKeyword,
    TrueKeyword,
    FalseKeyword,
    GreaterThan,
    LowerThan,
    Plus,
    Minus,
    Mult,
    Div,
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct Token {
    pub t: TokenType,
    pub value: Option<String>,
}

pub fn tokenize(input: &str) -> Result<Vec<Token>, Errors> {
    let mut tokens: Vec<Token> = Vec::new();
    let mut current_token: Option<Token> = None;
    for i in input.chars() {
        if tokens.last().is_some()
            && tokens.last().unwrap().t == TokenType::DoubleQuoteStart
            && i != '"'
        {
            if let Some(tok) = &mut current_token {
                match &mut tok.value {
                    Some(s) => s.push(i),
                    None => tok.value = Some(i.to_string()),
                }
            } else {
                current_token = Some(Token {
                    t: TokenType::StringLiteral,
                    value: Some(i.to_string()),
                })
            }
        } else if tokens.last().is_some() && tokens.last().unwrap().t == TokenType::DoubleQuoteEnd {
            if let Some(tok) = &mut current_token {
                match &mut tok.value {
                    Some(s) => s.push(i),
                    None => tok.value = Some(i.to_string()),
                }
            } else {
                current_token = Some(Token {
                    t: TokenType::StringLiteral,
                    value: Some(String::from(i.to_string())),
                });
            }
        } else if (i >= 'A' && i <= 'z' && i != '[' && i != ']')
            || i == '_'
            || (current_token.is_some()
                && current_token.clone().unwrap().t == TokenType::Ident
                && i != ' ')
        {
            if let Some(tok) = &mut current_token {
                match &mut tok.value {
                    Some(s) => {
                        s.push(i);
                        if s == "if" {
                            tokens.push(Token {
                                t: TokenType::IfKeyword,
                                value: None,
                            });
                            current_token = None;
                            continue;
                        } else if s == "for" {
                            tokens.push(Token {
                                t: TokenType::ForStatement,
                                value: None,
                            });
                            current_token = None;
                            continue;
                        }
                    }
                    None => tok.value = Some(i.to_string()),
                }
            } else {
                current_token = Some(Token {
                    t: TokenType::Ident,
                    value: Some(String::from(i.to_string())),
                });
            }
        } else if i == ';' {
            if let Some(tok) = &current_token {
                tokens.push(tok.clone());
            }
            tokens.push(Token {
                t: TokenType::SemiColon,
                value: None,
            });

            current_token = None;
        } else if i == '=' {
            if let Some(tok) = &mut current_token {
                tokens.push(tok.clone());
            }
            tokens.push(Token {
                t: TokenType::AssignOp,
                value: None,
            });
        } else if i == ' ' {
            if let Some(tok) = &mut current_token {
                tokens.push(tok.clone());
            }
            current_token = None;
        } else if i >= '0' && i <= '9' {
            if let Some(tok) = &mut current_token {
                match &mut tok.value {
                    Some(s) => s.push(i),
                    None => tok.value = Some(i.to_string()),
                }
            } else {
                current_token = Some(Token {
                    t: TokenType::Number,
                    value: Some(String::from(i.to_string())),
                });
            }
        } else if i == '"' {
            if current_token.is_some()
                && current_token.clone().unwrap().t != TokenType::StringLiteral
            {
                return Err(Errors::CannotCreateString);
            } else if current_token.is_some()
                && current_token.clone().unwrap().t == TokenType::StringLiteral
            {
                tokens.push(current_token.clone().unwrap());
                tokens.push(Token {
                    t: TokenType::DoubleQuoteEnd,
                    value: None,
                });
                current_token = None;
            } else {
                tokens.push(Token {
                    t: TokenType::DoubleQuoteStart,
                    value: None,
                })
            }
        } else if i == '[' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::SqBracketOpen,
                value: None,
            });
            current_token = None;
        } else if i == ']' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::SqBracketClose,
                value: None,
            });
            current_token = None;
        } else if i == '}' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::CuBracketOpen,
                value: None,
            });
            current_token = None;
        } else if i == '{' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::CuBracketClose,
                value: None,
            });
            current_token = None;
        } else if i == ',' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::Comma,
                value: None,
            });
            current_token = None;
        } else if i == '>' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::GreaterThan,
                value: None,
            });
            current_token = None;
        } else if i == '<' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::LowerThan,
                value: None,
            });
            current_token = None;
        } else if i == '+' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::Plus,
                value: None,
            });
            current_token = None;
        } else if i == '-' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::Minus,
                value: None,
            });
            current_token = None;
        } else if i == '*' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::Mult,
                value: None,
            });
            current_token = None;
        } else if i == '/' {
            if current_token.is_some() {
                tokens.push(current_token.clone().unwrap());
            }

            current_token = None;
            tokens.push(Token {
                t: TokenType::Div,
                value: None,
            });
            current_token = None;
        }
    }

    if let Some(tok) = current_token {
        tokens.push(tok)
    }
    Ok(tokens)
}
