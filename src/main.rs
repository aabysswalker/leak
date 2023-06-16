mod lexer;
mod parser;

fn main() {
    let token = lexer::tokenize(r#"i = 1"#).unwrap();

    let _parsed = parser::parse(token.clone());
}