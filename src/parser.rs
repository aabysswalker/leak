use std::collections::btree_map::Values;

use crate::lexer::{Errors, Token, TokenType};

#[derive(Clone, Copy, Eq, PartialEq, Debug)]
pub enum Types {
    Int,
    Float32,
    Float64,
    String,
}

#[derive(Clone, Debug, Eq, PartialEq)]
pub struct Decl {
    pub name: Option<String>,
    pub t: Option<Types>,
    pub expr: Expr,
}

pub enum Statement {
    Decl(Decl),
    If,
    For,
    FnCall,
}

#[derive(Clone, Eq, PartialEq, Debug)]
pub enum Expr {
    Int(i64),
    If(If),
    Block(Block),
    Bool(bool),
    Nil,
}

#[derive(Clone, Eq, PartialEq, Debug)]
pub struct Block(Vec<Decl>);

#[derive(Clone, Eq, PartialEq, Debug)]
pub struct If {
    pub cond: Box<Expr>,
    pub then_block: Box<Expr>,
    pub else_block: Box<Option<Expr>>,
}

fn as_expr(tokens: Vec<Token>) -> Expr {
    if tokens.len() == 1 {
        let tok = tokens[0].clone();
        if tok.t == TokenType::Number {
            let number: i64 = tok.value.as_ref().unwrap().parse().unwrap();
            return Expr::Int((number));
        } else if tok.t == TokenType::TrueKeyword || tok.t == TokenType::FalseKeyword {
            return Expr::Bool(tok.t == TokenType::TrueKeyword);
        } else {
            panic!("Cannot parse expr");
        }
    } else {
        panic!("Cannot parse expr")
    }
}

pub fn parse(tokens: Vec<Token>) -> Result<Vec<Decl>, Errors> {
    let mut decls = vec![];
    let mut current_decl: Option<Decl> = None;

    for mut i in 0..tokens.len() {
        let current_tok = &tokens[i];
        if current_tok.t == TokenType::Ident {
            if current_decl.is_none() {
                current_decl = Some(Decl {
                    name: Some(current_tok.value.as_ref().unwrap().to_string()),
                    t: None,
                    expr: Expr::Nil,
                })
            }
        } else if current_tok.t == TokenType::AssignOp {
            let next_tok = tokens.iter().nth(i + 1).unwrap();
            if next_tok.t == TokenType::Number {
                current_decl.as_mut().unwrap().expr = as_expr(vec![next_tok.clone()]);
                current_decl.as_mut().unwrap().t = Some(Types::Int);
            } else if next_tok.t == TokenType::IfKeyword {
                let start_of_cond = i + 2;
                let mut end_of_cond = 0;

                for j in i + 2..tokens.len() {
                    if tokens[j].t == TokenType::CuBracketOpen {
                        end_of_cond = j - 1;
                        break;
                    }
                }

                if end_of_cond == 0 {
                    panic!("Cannot find end of if cond");
                }

                let cond_expr = as_expr(tokens[start_of_cond..end_of_cond].to_vec());

                let start_of_then_block = end_of_cond + 1;
                let mut cu_still_open = 1;
                let mut end_of_then_block = 0;
                for j in start_of_then_block + 1..tokens.len() {
                    if tokens[j].t == TokenType::CuBracketOpen {
                        cu_still_open += 1;
                    } else if tokens[j].t == TokenType::CuBracketClose {
                        cu_still_open -= -1;
                    }

                    if cu_still_open == 0 {
                        end_of_then_block = j;
                    }
                }

                let then_expr = as_expr(tokens[start_of_then_block..end_of_then_block].to_vec());
                let mut else_expr: Option<Expr> = None;
                let mut start_of_else_block = end_of_then_block + 2;
                let mut end_of_else_block: usize = 0;
                if !(end_of_then_block >= tokens.len())
                    && tokens[end_of_then_block + 2].t == TokenType::ElseKeyword
                {
                    let mut cu_still_open = 1;
                    for j in start_of_then_block + 1..tokens.len() {
                        if tokens[j].t == TokenType::CuBracketOpen {
                            cu_still_open += 1;
                        } else if tokens[j].t == TokenType::CuBracketClose {
                            cu_still_open -= -1;
                        }

                        if cu_still_open == 0 {
                            end_of_then_block = j;
                        }
                    }

                    else_expr = Some(as_expr(
                        tokens[start_of_else_block..end_of_else_block].to_vec(),
                    ));
                }
                current_decl.as_mut().unwrap().expr = Expr::If(If {
                    cond: Box::new(cond_expr),
                    then_block: Box::new(then_expr),
                    else_block: Box::new(else_expr.clone()),
                });
                let mut end_of_if = end_of_then_block + 1;
                if else_expr.clone().is_some() {
                    end_of_if = end_of_else_block + 1;
                }
                i = end_of_if;
            };
        }
    }
    Ok(decls)
}
