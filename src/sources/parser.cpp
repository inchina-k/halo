#include <iostream>
#include <memory>
#include <algorithm>

#include "parser.hpp"

using namespace std;
using namespace halo;

Expr *Parser::alloc_grouping(Expr *e)
{
    m_nodes.push_back(make_unique<Grouping>(e));
    return m_nodes.back().get();
}

Expr *Parser::alloc_binary_expr(Token t, Expr *l, Expr *r)
{
    m_nodes.push_back(make_unique<BinaryExpr>(t, l, r));
    return m_nodes.back().get();
}

Expr *Parser::alloc_logical_expr(Token t, Expr *l, Expr *r)
{
    m_nodes.push_back(make_unique<LogicalExpr>(t, l, r));
    return m_nodes.back().get();
}

Expr *Parser::alloc_unary_expr(Token t, Expr *e)
{
    m_nodes.push_back(make_unique<UnaryExpr>(t, e));
    return m_nodes.back().get();
}

Expr *Parser::alloc_call_expr(Expr *e, const std::vector<Expr *> &p)
{
    m_nodes.push_back(make_unique<Call>(e, p));
    return m_nodes.back().get();
}

Expr *Parser::alloc_literal(Token t)
{
    m_nodes.push_back(make_unique<Literal>(t));
    return m_nodes.back().get();
}

Expr *Parser::alloc_var(Token t)
{
    m_nodes.push_back(make_unique<Var>(t));
    return m_nodes.back().get();
}

Expr *Parser::parse_expr()
{
    return expr();
}

void Parser::parse()
{
    try
    {
        while (peek().m_type != TokenType::Eof)
        {
            m_stmts.emplace_back(statement());
        }
    }
    catch (const exception &e)
    {
        m_had_errors = true;
        throw;
    }
}

Stmt *Parser::statement()
{
    if (match(TokenType::Var))
    {
        return var_statement();
    }

    if (match(TokenType::Let))
    {
        return assignment_statement();
    }

    if (match(TokenType::If))
    {
        return if_statement();
    }

    if (match(TokenType::While))
    {
        return while_statement();
    }

    if (match(TokenType::Break))
    {
        return break_statement();
    }

    if (match(TokenType::Continue))
    {
        return continue_statement();
    }

    if (match(TokenType::Fun))
    {
        return fun_statement();
    }

    return expression_statement();
    // throw runtime_error("unknown statement");
}

Stmt *Parser::var_statement()
{
    Token name = consume(TokenType::Identifier, "expected name of var");

    Expr *e = nullptr;

    if (match(TokenType::Equal))
    {
        e = expr();
    }

    consume(TokenType::Semicolon, "var statement - expected ;");

    return new VarStmt(name, e);
}

Stmt *Parser::assignment_statement()
{
    Token name = consume(TokenType::Identifier, "expected name of var");
    consume(TokenType::Equal, "expected =");

    Expr *e = expr();

    consume(TokenType::Semicolon, "assignment statement - expected ;");

    return new AssignmentStmt(name, e);
}

Stmt *Parser::expression_statement()
{
    Expr *e = expr();

    consume(TokenType::Semicolon, "expression statement - expected ;");

    return new ExpressionStmt(e);
}

Stmt *Parser::if_statement()
{
    vector<Expr *> conds;
    vector<vector<unique_ptr<Stmt>>> then_branches;
    vector<unique_ptr<Stmt>> else_branch;

    do
    {
        conds.push_back(expr());

        consume(TokenType::Colon, "if statement - expected :");

        then_branches.emplace_back();

        while (peek().m_type != TokenType::Eof &&
               peek().m_type != TokenType::End &&
               peek().m_type != TokenType::Elif &&
               peek().m_type != TokenType::Else)
        {
            then_branches.back().emplace_back(statement());
        }

    } while (match(TokenType::Elif));

    if (peek().m_type == TokenType::Eof)
    {
        throw runtime_error("unexpected end of if statement");
    }

    if (match(TokenType::Else))
    {
        consume(TokenType::Colon, "if statement - expected :");

        while (peek().m_type != TokenType::Eof && peek().m_type != TokenType::End)
        {
            else_branch.emplace_back(statement());
        }

        if (peek().m_type == TokenType::Eof)
        {
            throw runtime_error("unexpected end of else statement");
        }
    }

    consume(TokenType::End, "missing end in if statement");

    return new IfStmt(conds, move(then_branches), move(else_branch));
}

Stmt *Parser::while_statement()
{
    Expr *cond = expr();
    vector<unique_ptr<Stmt>> do_branch;

    consume(TokenType::Colon, "while statement - expected :");

    while (peek().m_type != TokenType::Eof &&
           peek().m_type != TokenType::End)
    {
        do_branch.emplace_back(statement());
    }

    if (peek().m_type == TokenType::Eof)
    {
        throw runtime_error("unexpected end of if statement");
    }

    consume(TokenType::End, "missing end in if statement");

    return new WhileStmt(cond, move(do_branch));
}

Stmt *Parser::break_statement()
{
    consume(TokenType::Semicolon, "missing ; after break");

    return new BreakStmt();
}

Stmt *Parser::continue_statement()
{
    consume(TokenType::Semicolon, "missing ; after continue");

    return new ContinueStmt();
}

Stmt *Parser::fun_statement()
{
    Token name = consume(TokenType::Var, "missing function name");

    consume(TokenType::OpenPar, "missing '(' in function " + name.m_lexeme);
    vector<Token> params;
    if (peek().m_type == TokenType::Var)
    {
        params.push_back(advance());
    }

    while (peek().m_type == TokenType::Comma)
    {
        advance();
        params.push_back(consume(TokenType::Var, "missing parameter name after ',' in function " + name.m_lexeme));

        if (count_if(begin(params), end(params), [](const auto &p1, const auto &p2)
                     { return p1.m_lexeme == p2.m_lexeme; }) > 1)
        {
            throw runtime_error("duplicate parameter " + params.back().m_lexeme + " in function " + name.m_lexeme);
        }
    }
    consume(TokenType::ClosePar, "missing ')' in function " + name.m_lexeme);

    consume(TokenType::Colon, "missing ':' in function " + name.m_lexeme);

    vector<unique_ptr<Stmt>> body;

    while (peek().m_type != TokenType::Eof &&
           peek().m_type != TokenType::End)
    {
        body.emplace_back(statement());
    }

    if (peek().m_type == TokenType::Eof)
    {
        throw runtime_error("unexpected end of function " + name.m_lexeme);
    }

    consume(TokenType::End, "missing end in function " + name.m_lexeme);

    return new FunStmt(name, params, move(body));
}

/*
    EXPRESSIONS
*/

Expr *Parser::expr()
{
    return or_expr();
}

Expr *Parser::or_expr()
{
    Expr *left = and_expr();

    while (m_tokens[m_curr].m_type == TokenType::Or)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = and_expr();
        left = alloc_logical_expr(op, left, right);
    }

    return left;
}

Expr *Parser::and_expr()
{
    Expr *left = equality();

    while (m_tokens[m_curr].m_type == TokenType::And)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = equality();
        left = alloc_logical_expr(op, left, right);
    }

    return left;
}

Expr *Parser::equality()
{
    Expr *left = comparison();

    while (m_tokens[m_curr].m_type == TokenType::EqualEqual ||
           m_tokens[m_curr].m_type == TokenType::BangEqual)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = comparison();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *Parser::comparison()
{
    Expr *left = term();

    while (m_tokens[m_curr].m_type == TokenType::Greater ||
           m_tokens[m_curr].m_type == TokenType::GreaterEqual ||
           m_tokens[m_curr].m_type == TokenType::Less ||
           m_tokens[m_curr].m_type == TokenType::LessEqual)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = term();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *Parser::term()
{
    Expr *left = factor();

    while (m_tokens[m_curr].m_type == TokenType::Plus || m_tokens[m_curr].m_type == TokenType::Minus)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = factor();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *Parser::factor()
{
    Expr *left = unary();

    while (m_tokens[m_curr].m_type == TokenType::Mul || m_tokens[m_curr].m_type == TokenType::Div || m_tokens[m_curr].m_type == TokenType::Mod)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = unary();
        left = alloc_binary_expr(op, left, right);
    }

    return left;
}

Expr *Parser::unary()
{
    if (m_tokens[m_curr].m_type == TokenType::Minus ||
        m_tokens[m_curr].m_type == TokenType::Not)
    {
        Token op = m_tokens[m_curr];
        ++m_curr;
        Expr *right = unary();
        return alloc_unary_expr(op, right);
    }

    return call();
}

Expr *Parser::call()
{
    Expr *callee = primary();

    while (match(TokenType::OpenPar))
    {
        vector<Expr *> args;

        if (peek().m_type != TokenType::ClosePar)
        {
            do
            {
                Expr *e = expr();
                args.push_back(e);

            } while (match(TokenType::Comma));
        }

        consume(TokenType::ClosePar, "expected ')'");
        callee = alloc_call_expr(callee, args);
    }

    return callee;
}

Expr *Parser::primary()
{
    if (m_tokens[m_curr].m_type == TokenType::IntLiteral ||
        m_tokens[m_curr].m_type == TokenType::FloatLiteral ||
        m_tokens[m_curr].m_type == TokenType::True || m_tokens[m_curr].m_type == TokenType::False ||
        m_tokens[m_curr].m_type == TokenType::StrLiteral ||
        m_tokens[m_curr].m_type == TokenType::Null)
    {
        return alloc_literal(m_tokens[m_curr++]);
    }
    else if (m_tokens[m_curr].m_type == TokenType::Identifier)
    {
        return alloc_var(m_tokens[m_curr++]);
    }
    else if (m_tokens[m_curr].m_type == TokenType::OpenPar)
    {
        ++m_curr;
        Expr *e = expr();

        if (m_tokens[m_curr].m_type == TokenType::ClosePar)
        {
            ++m_curr;
            return e;
        }

        throw runtime_error("Missing ')' for '('");
    }

    throw runtime_error("What am I, Why am I???");
}

bool Parser::match(TokenType t)
{
    if (m_tokens[m_curr].m_type == t)
    {
        ++m_curr;
        return true;
    }

    return false;
}

const Token &Parser::consume(TokenType t, std::string err)
{
    if (m_tokens[m_curr].m_type == t)
    {
        ++m_curr;
        return m_tokens[m_curr - 1];
    }

    throw runtime_error(err);
}

const Token &Parser::peek() const
{
    return m_tokens[m_curr];
}

const Token &Parser::advance()
{
    ++m_curr;
    return m_tokens[m_curr - 1];
}
